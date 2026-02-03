// 

#include "FlowGraph/Nodes/FlowNode_NpcGoal.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "FlowAsset.h"
#include "AddOns/FlowNodeAddOn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "FlowGraph/Addons/FlowNodeAddon_ActivityEQS.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"

UFlowNode_NpcGoal::UFlowNode_NpcGoal(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InputPins = { TEXT("Start"), TEXT("Suspend") };
	OutputPins = { TEXT("Started"), TEXT("Resumed"), TEXT("Suspended"), TEXT("Completed"), TEXT("Aborted"), TEXT("Failed") };
	
#if WITH_EDITOR
	Category = TEXT("NPC");
	NodeDisplayStyle = FlowNodeStyle::Latent;
#endif
}

void UFlowNode_NpcGoal::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: execute input %s"), *GetName(), *PinName.ToString());
	ENpcGoalStartResult Result = ENpcGoalStartResult::None;
	
	if (PinName == "Start")
	{
		bool bResuming = CurrentGoalState == EGoalState::Suspended;
		Result = Start();
	
		switch (Result)
		{
			case ENpcGoalStartResult::InProgress:
			UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: execute input -> in progress"), *GetName());

				if (!bResuming)
				{
					NpcFlowComponent->OnGoalStarted(this);
					TriggerOutput(FName("Started"));
				}
				else
				{
					TriggerOutput(FName("Resumed"));
				}
				break;
			case ENpcGoalStartResult::Failed:
				UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: execute input -> failed"), *GetName());
				TriggerOutput(FName("Failed"), true);
				break;
			case ENpcGoalStartResult::Finished:
				TriggerOutput(FName("Completed"), true);
				break;
			case ENpcGoalStartResult::None:
			default:
				ensure(false);
				TriggerOutput(FName("Failed"), true);
				break;
		}
	}
	else if (PinName == "Suspend" && CurrentGoalState == EGoalState::Running)
	{
		UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: execute input -> suspend"), *GetName());
		Suspend();
		TriggerOutput(FName("Suspended"));
	}
}

void UFlowNode_NpcGoal::InitializeInstance()
{
	Super::InitializeInstance();
	auto OwnerController = Cast<AAIController>(TryGetRootFlowActorOwner());
	BlackboardComponent = OwnerController->GetBlackboardComponent();
	NpcPawn = OwnerController->GetPawn();
	NpcComponent = NpcPawn->GetComponentByClass<UNpcComponent>();
	NpcFlowComponent = TryGetRootFlowActorOwner()->FindComponentByClass<UNpcFlowComponent>();
	BlackboardKeys = NpcComponent->GetNpcBlackboardKeys();
	for (const auto& Addon : AddOns)
	{
		if (auto ActivityEQSProvider = Cast<UFlowNodeAddon_ActivityEQS>(Addon.Get()))
		{
			ActivityEQSProviderAddon = ActivityEQSProvider;
			break;
		}
	}
}

FEQSParametrizedQueryExecutionRequest* UFlowNode_NpcGoal::GetEQSRequest(const FGameplayTag& Tag)
{
	return  ActivityEQSProviderAddon.IsValid() ? ActivityEQSProviderAddon->GetEQSRequest(Tag) : nullptr;
}

ENpcGoalStartResult UFlowNode_NpcGoal::Start()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFlowNode_NpcGoal::Start)

	ClearBlackboard();
	
	BlackboardComponent->SetValueAsBool(BlackboardKeys->RequestResetGoalBBKey.SelectedKeyName, true);

	ENpcGoalStartResult Result = Restore(CurrentGoalState == EGoalState::Inactive);

	return Result;
}

ENpcGoalStartResult UFlowNode_NpcGoal::Restore(bool bInitialStart)
{
	if (!BlackboardKeys->GoalTagsBBKey.SelectedKeyName.IsNone())
		BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GoalTagsBBKey.SelectedKeyName, CustomGoalTags);

	BlackboardComponent->SetValueAsEnum(BlackboardKeys->GoalTypeBBKey.SelectedKeyName, (uint8)NpcGoalType);
	
	if (bRunIndefinitely)
	{
		BlackboardComponent->ClearValue(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName);
	}
	else
	{
		if (bInitialStart)
		{
			auto NpcGameMode = Cast<INpcSystemGameMode>(BlackboardComponent->GetWorld()->GetAuthGameMode());
			if (!ensure(NpcGameMode))
				return ENpcGoalStartResult::Failed;
	
			const float GameTimeDuration = NpcGameMode->ConvertGameTimeToRealTime(GameTimeDurationInHours);
			RemainingGoalExecutionTime = FMath::RandRange(GameTimeDuration * (1.f - RandomTimeDeviation),GameTimeDuration * (1.f + RandomTimeDeviation));
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName, GameTimeDuration);
		}
		else if (RemainingGoalExecutionTime > 0.f)
		{
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName, RemainingGoalExecutionTime);
		}
	}
		
	BlackboardComponent->SetValueAsBool(BlackboardKeys->RunIndefinitelyBBKey.SelectedKeyName, bRunIndefinitely);

	CurrentGoalState = EGoalState::Running;
	return ENpcGoalStartResult::InProgress;
}

ENpcGoalAdvanceResult UFlowNode_NpcGoal::Advance(const FGameplayTagContainer& GoalExecutionResultTags)
{
	UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: advance"), *GetName());
	return GoalExecutionResultTags.IsEmpty() || !GoalExecutionResultTags.HasTagExact(AIGameplayTags::Activity_Goal_Result_Execution_Failure)
		? ENpcGoalAdvanceResult::Completed
		: ENpcGoalAdvanceResult::Failed;
}

void UFlowNode_NpcGoal::Finish()
{
	UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: finish"), *GetName());
	Suspend();
	CurrentGoalState = EGoalState::Inactive;
	NpcFlowComponent->OnGoalEnded(this);
	// BlackboardComponent->SetValueAsBool(BlackboardKeys->RequestResetGoalBBKey.SelectedKeyName, true);
	Super::Finish();
}

void UFlowNode_NpcGoal::RequestResumeGoal()
{
	UE_VLOG(TryGetRootFlowActorOwner(), LogARPGAI_Activity, Verbose, TEXT("%s: request resume goal"), *GetName());
	ClearBlackboard();
	Restore(false);
}

void UFlowNode_NpcGoal::RequestExternalAbort()
{
	UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: request external abort"), *GetName());
	TriggerOutput("Aborted", true);
}

void UFlowNode_NpcGoal::RequestLoadState()
{
	UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: request load state"), *GetName());
	ClearBlackboard();
	Restore(false);
}

void UFlowNode_NpcGoal::ClearBlackboard()
{
	BlackboardComponent->ClearValue(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->RunIndefinitelyBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->GoalTypeBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->EqsToRunBBKey.SelectedKeyName);

	BlackboardComponent->ClearValue(BlackboardKeys->LocationToGoBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->NpcActivityStateBBKey.SelectedKeyName);

}

void UFlowNode_NpcGoal::Suspend()
{
	if (!bRunIndefinitely)
	{
		if (ensure(!BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName.IsNone()))
			RemainingGoalExecutionTime = BlackboardComponent->GetValueAsFloat(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName);
	}

	CurrentGoalState = EGoalState::Suspended;
}

FFlowDataPinResult_GameplayTagContainer UFlowNode_NpcGoal::TrySupplyDataPinAsGameplayTagContainer_Implementation(
	const FName& PinName) const
{
	static const FName OUTPIN_GameplayTagContainerOutput = GET_MEMBER_NAME_CHECKED(UFlowNode_NpcGoal, OutGoalExecutionResultTags);

	FFlowDataPinResult_GameplayTagContainer Result;
	if (PinName == OUTPIN_GameplayTagContainerOutput)
	{
		Result = FFlowDataPinResult_GameplayTagContainer(OutGoalExecutionResultTags);
	}
	else
	{
		Result = Super::TrySupplyDataPinAsGameplayTagContainer_Implementation(PinName);
	}

	LogNote(FString::Printf(TEXT("%s supplied %s for pin %s"), *GetName(), *Result.Value.ToStringSimple(), *PinName.ToString()));

	return Result;
}

ENpcGoalAdvanceResult UFlowNode_NpcGoal::RequestAdvanceGoal(const FGameplayTagContainer& GoalExecutionResultTags)
{
	UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: request advance goal"), *GetName());
	auto Result = Advance(GoalExecutionResultTags);
	switch (Result)
	{
		case ENpcGoalAdvanceResult::InProgress:
			UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: request advance goal -> InProgress"), *GetName());
			break;
		case ENpcGoalAdvanceResult::Completed:
			UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: request advance goal -> Completed"), *GetName());
			OutGoalExecutionResultTags = GoalExecutionResultTags;
			TriggerOutput(FName("Completed"), true);
			NpcFlowComponent->OnGoalCompleted(CustomGoalTags, GoalExecutionResultTags);
			break;
		case ENpcGoalAdvanceResult::Failed:
			UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: request advance goal -> Failed"), *GetName());
			TriggerOutput(FName("Failed"), true);
			break;
		default:
			ensure(false);
			TriggerOutput(FName("Failed"), true);
			break;
	}

	return Result;
}

void UFlowNode_NpcGoal::RequestSuspendGoal()
{
	UE_VLOG(NpcPawn.Get(), LogARPGAI_Activity, Verbose, TEXT("%s: request suspend goal"), *GetName());
	Suspend();
}

#if WITH_EDITOR

EDataValidationResult UFlowNode_NpcGoal::ValidateNode()
{
	auto BaseResult = Super::ValidateNode();
	if (BaseResult == EDataValidationResult::Invalid)
		return BaseResult;

	return EDataValidationResult::Valid;
}

FString UFlowNode_NpcGoal::GetGoalDescription() const
{
	FString Description;
	if (bRunIndefinitely)
	{
		Description += TEXT("Run indefinitely");
	}
	else 
	{
		const float Min = GameTimeDurationInHours * (1.f - RandomTimeDeviation);
		const float Max = GameTimeDurationInHours * (1.f + RandomTimeDeviation);
		Description += FString::Printf(TEXT("Run for [%.2f -> %.2f] game hours"), Min, Max);
	}

	if (!CustomGoalTags.IsEmpty())
		Description += FString::Printf(TEXT("\nGoal tags:\n%s"), *CustomGoalTags.ToString());
	
	return Description;
}

FText UFlowNode_NpcGoal::GetNodeConfigText() const
{
	FString Description = GetGoalDescription();
	return FText::FromString(Description + TEXT("\n") + ExtraUserDescription);
}

FString UFlowNode_NpcGoal::GetStatusString() const
{
	return TEXT("NpcGoal: test status string");
}

#endif
