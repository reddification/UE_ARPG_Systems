#include "FlowGraph/Addons/FlowNodeAddon_ActivityParameters.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "FlowGraph/Nodes/FlowNode_NpcGoal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcControllerInterface.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Interfaces/NpcZone.h"
#include "Nodes/FlowNode_Transit.h"
#include "Nodes/Route/FlowNode_Branch.h"
#include "Nodes/Route/FlowNode_ExecutionMultiGate.h"
#include "Nodes/Route/FlowNode_ExecutionSequence.h"

EFlowAddOnAcceptResult UFlowNodeAddon_ActivityParameters::AcceptFlowNodeAddOnParent_Implementation(
	const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	bool bFits = ParentTemplate->IsA<UFlowNode_NpcGoal>() || ParentTemplate->IsA<UFlowNode_Branch>()
		|| ParentTemplate->IsA<UFlowNode_ExecutionSequence>() || ParentTemplate->IsA<UFlowNode_ExecutionMultiGate>()
		|| ParentTemplate->IsA<UFlowNode_Transit>();
	
	return bFits ? EFlowAddOnAcceptResult::TentativeAccept : EFlowAddOnAcceptResult::Reject;
}

void UFlowNodeAddon_SetActivityLocations::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	if (ActivityLocations.IsEmpty())
		return;

	auto Controller = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto Pawn = Controller->GetPawn();
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	auto NpcAreas = NpcGameMode->GetNpcAreas(ActivityLocations, Pawn);
	auto NpcAreasComponent = Pawn->FindComponentByClass<UNpcAreasComponent>();
	
	for (auto* AreaActor : NpcAreas)
	{
		TScriptInterface<INpcZone> NpcArea;
		NpcArea.SetObject(AreaActor);
		NpcArea.SetInterface(Cast<INpcZone>(AreaActor));
		NpcAreasComponent->AddAreaOfInterest(AIGameplayTags::Location_Activity, NpcArea);
	}

	if (bOverwriteActivityLocations)
		NpcAreasComponent->SetExclusiveNpcAreaType(AIGameplayTags::Location_Activity);
}

void UFlowNodeAddon_SetActivityLocations::FinishState()
{
	Super::FinishState();
	if (ActivityLocations.IsEmpty())
		return;
	
	auto Controller = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto Pawn = Controller->GetPawn();
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	auto NpcAreas = NpcGameMode->GetNpcAreas(ActivityLocations, Pawn);
	auto NpcAreasComponent = Controller->GetPawn()->FindComponentByClass<UNpcAreasComponent>();

	for (auto* AreaActor : NpcAreas)
	{
		TScriptInterface<INpcZone> NpcArea;
		NpcArea.SetObject(AreaActor);
		NpcArea.SetInterface(Cast<INpcZone>(AreaActor));
		NpcAreasComponent->RemoveNpcArea(AIGameplayTags::Location_Activity, NpcArea);
	}
	
	if (bOverwriteActivityLocations)
		NpcAreasComponent->RemoveExclusiveNpcAreaType(AIGameplayTags::Location_Activity);
}

#if WITH_EDITOR

FText UFlowNodeAddon_SetActivityLocations::GetNodeConfigText() const
{
	if (ActivityLocations.IsEmpty())
		return FText::FromString(bOverwriteActivityLocations ? TEXT("Clear activity locations") : TEXT("Warning: activity locations not set"));
	
	return FText::FromString(FString::Printf(TEXT("%s\n%s"), bOverwriteActivityLocations ? TEXT("Overwrite") : TEXT("Append"),
		*ActivityLocations.ToStringSimple()));
}

#endif

void UFlowNodeAddon_SetCharacterState::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	if (ensure(CharacterStateTag.IsValid()))
	{
		auto Controller = Cast<AAIController>(TryGetRootFlowActorOwner());
		auto BlackboardKeys = Controller->GetPawn()->FindComponentByClass<UNpcComponent>()->GetNpcBlackboardKeys();
		Controller->GetBlackboardComponent()->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->NpcActivityStateBBKey.SelectedKeyName, 
			CharacterStateTag.GetSingleTagContainer());
	}
}

void UFlowNodeAddon_SetCharacterState::FinishState()
{
	if (ensure(CharacterStateTag.IsValid()))
	{
		auto Controller = Cast<AAIController>(TryGetRootFlowActorOwner());
		auto BlackboardKeys = Controller->GetPawn()->FindComponentByClass<UNpcComponent>()->GetNpcBlackboardKeys();
		Controller->GetBlackboardComponent()->ClearValue(BlackboardKeys->NpcActivityStateBBKey.SelectedKeyName);
	}
	
	Super::FinishState();
}

void UFlowNodeAddon_SetAttitudePreset::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	if (!AttitudePresetTag.IsValid())
		return;

	auto Controller = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto NpcComponent = Controller->GetPawn()->FindComponentByClass<UNpcAttitudesComponent>();
	NpcComponent->SetAttitudePreset(AttitudePresetTag);
}

void UFlowNodeAddon_SetAttitudePreset::FinishState()
{
	if (!AttitudePresetTag.IsValid())
		return;

	auto Controller = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto NpcComponent = Controller->GetPawn()->FindComponentByClass<UNpcAttitudesComponent>();
	NpcComponent->ClearAttitudePreset();
	
	Super::FinishState();
}

void UFlowNodeAddon_SetNavigationFilterClass::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	if (!ensure(!NavigationFilterClass.IsNull()))
		return;

	auto Controller = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto NpcController = Cast<INpcControllerInterface>(Controller);
	ensure(!IsValid(InitialNavigationFilterClass)); // otherwise it means the addon is not instantiated and hence it's not allowed to store variables
	InitialNavigationFilterClass = NpcController->GetNpcDefaultNavigationFilterClass();
	NpcController->SetNavigationFilterClass(NavigationFilterClass.LoadSynchronous());
}

void UFlowNodeAddon_SetNavigationFilterClass::FinishState()
{
	if (NavigationFilterClass.IsNull())
		return;

	auto Controller = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto NpcController = Cast<INpcControllerInterface>(Controller);
	NpcController->SetNavigationFilterClass(InitialNavigationFilterClass);
	InitialNavigationFilterClass = nullptr;
	
	Super::FinishState();
}

void UFlowNodeAddon_RequestBehaviorEvaluators::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	auto AIController = TryGetRootFlowActorOwner();
	if (auto BehaviorEvaluatorComponent = AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent>())
	{
		if (bRequestActive)
			BehaviorEvaluatorComponent->RequestEvaluatorsActive(BehaviorEvaluatorsTags, true);
		else
			BehaviorEvaluatorComponent->RequestEvaluatorsActive(BehaviorEvaluatorsTags, true);
	}
}

void UFlowNodeAddon_RequestBehaviorEvaluators::FinishState()
{
	auto AIController = TryGetRootFlowActorOwner();
	if (IsValid(AIController))
	{
		if (auto BehaviorEvaluatorComponent = AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent>())
		{
			if (bRequestActive)
				BehaviorEvaluatorComponent->RequestEvaluatorsActive(BehaviorEvaluatorsTags, false);
			else
				BehaviorEvaluatorComponent->RequestEvaluatorsActive(BehaviorEvaluatorsTags, false);
		}
	}
	
	Super::FinishState();
}

#if WITH_EDITOR

FText UFlowNodeAddon_RequestBehaviorEvaluators::GetNodeConfigText() const
{
	return BehaviorEvaluatorsTags.IsEmpty()
		? FText::FromString(TEXT("No behavior evaluators provided"))
		: FText::FromString(FString::Printf(TEXT("%s behavior evaluators:\n%s"), bRequestActive ? TEXT("Request") : TEXT("Block"), *BehaviorEvaluatorsTags.ToStringSimple()));
}

#endif