#include "Components/Controller/NpcActivityComponent.h"

#include "AIController.h"
#include "Activities/NpcGoals.h"
#include "Data/NpcDTR.h"
#include "Data/NpcSettings.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcControllerInterface.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Subsystems/NpcActivitySquadSubsystem.h"

void UNpcActivityComponent::InitializeNpc(ACharacter* InNpc, const FNpcDTR* NpcDTR)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcActivityComponent::InitializeNpc)
	
	NpcComponent = InNpc->FindComponentByClass<UNpcComponent>();
	if (!ensure(NpcComponent.IsValid() && NpcDTR)) return;

	auto AIController = Cast<AAIController>(InNpc->GetOwner());
	NpcController.SetObject(InNpc->GetController());
	NpcController.SetInterface(Cast<INpcControllerInterface>(InNpc->GetController()));

	auto AliveCreature = Cast<INpcAliveCreature>(InNpc);
	AliveCreature->OnDeathStarted.AddUObject(this, &UNpcActivityComponent::OnNpcDied);

	Npc.SetObject(InNpc);
	Npc.SetInterface(Cast<INpc>(InNpc));
	
	BlackboardComponent = Cast<UBehaviorTreeComponent>(AIController->GetBrainComponent())->GetBlackboardComponent();

	// @AK 20.05.2025: THIS IS BAD! NpcID should be taken from the NpcComponent (and NpcComponent takes it from its owner, and its owner takes it from the DTRH)
	// however, despite NpcComponent->GetNpcIdTag() and NpcDTR->NpcId being equal now, there's no guarantee it will always be this way
	// the reason why I'm leaving this "hack" here for now is that the NPC initialization is fucked up and needs some proper refactoring
	// I tried delaying it for 1 frame in NpcController (which calls this method), however it then fucked up something with the quest system IIRC
	const auto& NpcId = NpcDTR->NpcId;
	// const auto& NpcId = NpcComponent->GetNpcIdTag();
	if (ensure(NpcId.IsValid()))
		GetWorld()->GetSubsystem<UNpcActivitySquadSubsystem>()->RegisterNpc(NpcId, this);
	
	DayTimeTag = NpcController->GetDayTime();
	// GetWorld()->GetTimerManager().SetTimerForNextTick([this](){ RunActivity(NpcComponent->GetNpcDTR(), DayTimeTag); });

	if (auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode()))
	{
		// TODO i'm not sure this is correct math lol
		GameTimeToRealTimeCoefficient = 3600.f / NpcGameMode->GetTimeRateSeconds();
	}
}

void UNpcActivityComponent::SetDayTime(const FGameplayTag& InDayTimeTag)
{
	if (CurrentActivityOriginType == EActivityOriginType::Lifecycle || CurrentActivityOriginType == EActivityOriginType::None)
	{
		DayTimeTag = InDayTimeTag;
		if (CurrentActivity)
		{
			float Delay = FMath::RandRange(MinDailyActivityDelayGameTime * GameTimeToRealTimeCoefficient, MaxDailyActivityDelayGameTime * GameTimeToRealTimeCoefficient);
			GetWorld()->GetTimerManager().SetTimer(DailyActivityDelayTimer, this, &UNpcActivityComponent::StartDelayedDailyActivity, Delay);
		}
		else
		{
			// Some activities have subordinates, but at BeginPlay it can happen that not all NPCs have registered in NPC subsystem, so we have to delay running 1st activity
			GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				if (this->CurrentActivityOriginType != EActivityOriginType::Quest && this->CurrentActivityOriginType != EActivityOriginType::Subordinate)
					RunActivity(NpcComponent->GetNpcDTR(), DayTimeTag);
			});
		}
	}
}

void UNpcActivityComponent::OnNpcInteractionStateChanged(AActor* InteractionInstigator, const FGameplayTag& InteractionTypeTag, bool bActive)
{
	if (InteractionTypeTag == AIGameplayTags::AI_Interaction_Dialogue)
	{
		if (bActive)
		{
			EnterDialogue(InteractionInstigator);
		}
		else
		{
			// if (ActiveNpcGoal.NpcGoal && ActiveNpcGoal.NpcGoal->NpcGoalType == ENpcGoalType::TalkToPlayer)
			// {
				// I don't think it's correct to do send AI message here. TODO refactor
				auto AIController = Cast<AAIController>(GetOwner());
				FAIMessage AIMessage;
				AIMessage.Status = FAIMessage::Success;
				AIMessage.MessageName = AIGameplayTags::AI_BrainMessage_Dialogue_Player_Completed.GetTag().GetTagName();
				AIController->GetBrainComponent()->HandleMessage(AIMessage);
			// }

			ExitDialogue();
		}
	}
}

void UNpcActivityComponent::RunCustomBehavior(const FGameplayTag& CustomBehaviorTag)
{
	CurrentActivityOriginType = EActivityOriginType::Quest;
	RunActivity(NpcComponent->GetNpcDTR(), CustomBehaviorTag);
}

void UNpcActivityComponent::StopCustomBehavior()
{
	CurrentActivityOriginType = EActivityOriginType::Lifecycle;
	const FGameplayTag& CurrentDayTime = NpcController->GetDayTime();
	RunActivity(NpcComponent->GetNpcDTR(), CurrentDayTime);
}

void UNpcActivityComponent::StartActivity(const FNpcDTR* NpcDTR, UNpcActivityDataAsset* NewActivity)
{
	if (CurrentActivity == NewActivity)
		return;
	
	StartNewActivity(NewActivity);
	ResetNavigationFilterClass();
	if (IsValid(NewActivity->ActivityNavigationFilterClass))
		SetNavigationFilterClass(NewActivity->ActivityNavigationFilterClass);
	
	BlackboardComponent->SetValueAsBool(NpcDTR->NpcBlackboardDataAsset->RequestResetGoalBBKey.SelectedKeyName, true);
}

void UNpcActivityComponent::RunActivity(const FNpcDTR* NpcDTR, const FGameplayTag& ActivityTag)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcActivityComponent::RunActivity)
	
	const FNpcActivityParametersOptions* NpcActivityParametersOptions = NpcDTR->ActivityDefinitions.Find(ActivityTag);
	if (NpcActivityParametersOptions == nullptr || NpcActivityParametersOptions->NpcActivityOptions.IsEmpty())
	{
		UE_VLOG(GetOwner(), LogNpcActivity, Warning, TEXT("Can't start activity %s, no activity sattisfying world state. Running default activity"), *ActivityTag.ToString());
		StartActivity(NpcDTR, NpcDTR->DefaultActivityDataAsset);
		return;
	}
	
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetOwner()->GetWorld()->GetAuthGameMode());
	if (!ensure(NpcGameMode))
		return;
	
	TArray<const FNpcActivityOption*> PossibleActivityDefinitions;
	PossibleActivityDefinitions.Reserve(NpcActivityParametersOptions->NpcActivityOptions.Num());

	FGameplayTagContainer NpcTags = Npc->GetNpcOwnerTags();
	for (const auto& ActivityDefinition : NpcActivityParametersOptions->NpcActivityOptions)
	{
		const bool bPassesWorldState = NpcGameMode->IsWorldAtState(ActivityDefinition.OnlyAtWorldState);
		const bool bPassesCharacterState = ActivityDefinition.OnlyAtCharacterState.IsEmpty() || ActivityDefinition.OnlyAtCharacterState.Matches(NpcTags);
		if (bPassesCharacterState && bPassesWorldState)
			PossibleActivityDefinitions.Emplace(&ActivityDefinition);
	}
	
	if (PossibleActivityDefinitions.IsEmpty())
	{
		UE_VLOG(GetOwner(), LogNpcActivity, Warning, TEXT("Can't start activity %s, no activity sattisfying world state. Running default activity"), *ActivityTag.ToString());
		StartActivity(NpcDTR, NpcDTR->DefaultActivityDataAsset);
		return;
	}
	
	const FNpcActivityOption* ActivityDefinitionToRun = PossibleActivityDefinitions[FMath::RandRange(0, PossibleActivityDefinitions.Num() - 1)];

	// don't start new activity if it's the same as current
	// this is important for patrolling NPCs for example, so that they don't just stop, disperse and then immediately gather together and continue patrolling
	auto* NewActivity = ActivityDefinitionToRun->ActivityDataAsset;
	UE_VLOG(GetOwner(), LogNpcActivity, Log, TEXT("Starting new activity %s"), *ActivityTag.ToString());
	StartActivity(NpcDTR, NewActivity);
}

void UNpcActivityComponent::SetNavigationFilterClass(const TSubclassOf<UNavigationQueryFilter>& CustomNavigationFilter)
{
	InitialNavigationFilterClass = NpcController->GetNpcDefaultNavigationFilterClass();
	NpcController->SetNavigationFilterClass(CustomNavigationFilter);
}

void UNpcActivityComponent::ResetNavigationFilterClass()
{
	if (IsValid(InitialNavigationFilterClass))
	{
		NpcController->SetNavigationFilterClass(InitialNavigationFilterClass);
	}
}

void UNpcActivityComponent::EnterDialogue(AActor* InteractionInstigator)
{
	auto NpcDTR = NpcComponent->GetNpcDTR();
	if (!ensure(NpcDTR) || !ensure(NpcDTR->NpcBlackboardDataAsset) || !ensure(!NpcDTR->NpcBlackboardDataAsset->ConversationPartnerBBKey.SelectedKeyName.IsNone()))
		return;
	
	BlackboardComponent->SetValueAsObject(NpcDTR->NpcBlackboardDataAsset->ConversationPartnerBBKey.SelectedKeyName, InteractionInstigator);
	// setting bAcceptedConversation switches the execution branch
	BlackboardComponent->SetValueAsBool(NpcDTR->NpcBlackboardDataAsset->bAcceptedConversationBBKey.SelectedKeyName, true);
}

void UNpcActivityComponent::ExitDialogue()
{
	auto NpcDTR = NpcComponent->GetNpcDTR();
	if (!ensure(NpcDTR) || !ensure(NpcDTR->NpcBlackboardDataAsset) || !ensure(!NpcDTR->NpcBlackboardDataAsset->ConversationPartnerBBKey.SelectedKeyName.IsNone()))
		return;
	
	BlackboardComponent->ClearValue(NpcDTR->NpcBlackboardDataAsset->ConversationPartnerBBKey.SelectedKeyName);
	BlackboardComponent->SetValueAsBool(NpcDTR->NpcBlackboardDataAsset->bAcceptedConversationBBKey.SelectedKeyName, false);
}

FVector UNpcActivityComponent::GetPawnLocation() const
{
	return NpcComponent->GetOwner()->GetActorLocation();
}

void UNpcActivityComponent::BeginPlay()
{
	Super::BeginPlay();
	auto NpcSettings = GetNpcSettings();
	InteractionsInterruptedByDialogue = NpcSettings->InteractionsInterruptedByDialogue;
	InterruptCurrentActivityForDialogueWhenNpcInStateFilter = NpcSettings->InterruptCurrentActivityForDialogueWhenNpcInStateFilter;
}

void UNpcActivityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (NpcComponent.IsValid())
		if (auto World = GetWorld())
			if (auto NpcSubsystem = World->GetSubsystem<UNpcActivitySquadSubsystem>())
				NpcSubsystem->UnregisterNpc(NpcComponent->GetNpcIdTag(), this);

	if (EndPlayReason != EEndPlayReason::RemovedFromWorld)
	{
		for (const auto& GoalMemory : GoalsMemories)
			delete GoalMemory.Value;

		GoalsMemories.Reset();
	}
	
	Super::EndPlay(EndPlayReason);
}

const UNpcSettings* UNpcActivityComponent::GetNpcSettings() const
{
	return StaticCast<const UNpcSettings*>(UNpcSettings::StaticClass()->GetDefaultObject());
}

void UNpcActivityComponent::OnNpcDied(AActor* DeadNpc)
{
	GetWorld()->GetSubsystem<UNpcActivitySquadSubsystem>()->UnregisterNpc(NpcComponent->GetNpcIdTag(), this);
}

void UNpcActivityComponent::RequestRestartBehaviorTreeGoalExecution()
{
	if (auto NpcDTR = NpcComponent->GetNpcDTR())
	{
		if (NpcDTR->NpcBlackboardDataAsset)
			BlackboardComponent->SetValueAsBool(NpcDTR->NpcBlackboardDataAsset->RequestResetGoalBBKey.SelectedKeyName, true);
	}
}

void UNpcActivityComponent::OnNpcReachedLocation(const FGameplayTagContainer& WorldLocationTags)
{
	if (auto NpcGoalVisitLocation = Cast<UNpcGoalVisitLocation>(ActiveNpcGoal.NpcGoal))
	{
		if (NpcGoalVisitLocation->bCompleteOnEntering && WorldLocationTags.HasTagExact(NpcGoalVisitLocation->LocationIdTag))
		{
			RequestNextNpcGoal();
			RequestRestartBehaviorTreeGoalExecution();
		}
	}
}

void UNpcActivityComponent::StartDelayedDailyActivity()
{
	RunActivity(NpcComponent->GetNpcDTR(), DayTimeTag);
}

void UNpcActivityComponent::StartNewActivity(const UNpcActivityDataAsset* InActivityParameters)
{
	CurrentActivity = InActivityParameters;
	NpcGoalIndex = -1;
	ActiveNpcGoalChainIndex = -1;
	
	RequestNextNpcGoal();
}

const FGameplayTag& UNpcActivityComponent::GetActivityStateTag() const
{
	return CurrentActivity->DefaultActivityNpcState;
}

bool UNpcActivityComponent::SetActivityGoalData()
{
	if (!ActiveNpcGoal.NpcGoal)
	{
		UE_VLOG(GetOwner(), LogNpcActivity, Warning, TEXT("UNpcActivityComponent::SetActivityGoalData - no active goal"));
		return false;
	}
	
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcActivityComponent::SetActivityGoalData)
	
	auto BlackboardKeys = NpcComponent->GetNpcDTR()->NpcBlackboardDataAsset;
	ENpcGoalStartResult StartResult = !bGoalAssessed
		? ActiveNpcGoal.NpcGoal->Start(BlackboardComponent.Get(), BlackboardKeys, this)
		: ActiveNpcGoal.NpcGoal->Restore(BlackboardComponent.Get(), BlackboardKeys, this, false);
	
	if (StartResult == ENpcGoalStartResult::InProgress)
	{
		if (CurrentActivityOriginType != EActivityOriginType::Subordinate)
		{
			if (bGoalAssessed)
			{
				BlackboardComponent->SetValueAsFloat(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName, RemainingGoalExecutionTime);
			}
			else
			{
				if (ActiveNpcGoal.bRunIndefinitely)
				{
					BlackboardComponent->ClearValue(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName);
				}
				else
				{
					auto NpcGameMode = Cast<INpcSystemGameMode>(BlackboardComponent->GetWorld()->GetAuthGameMode());
					if (!ensure(NpcGameMode))
						return false;
		
					const float GameTimeDuration = ActiveNpcGoal.GameTimeDurationInHours * 3600.f / NpcGameMode->GetTimeRateSeconds();
					RemainingGoalExecutionTime = FMath::RandRange(GameTimeDuration * (1.f - ActiveNpcGoal.RandomTimeDeviation),
						GameTimeDuration * (1.f + ActiveNpcGoal.RandomTimeDeviation));
					BlackboardComponent->SetValueAsFloat(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName, GameTimeDuration);
				}

				BlackboardComponent->SetValueAsBool(BlackboardKeys->RunIndefinitelyBBKey.SelectedKeyName, ActiveNpcGoal.bRunIndefinitely);
			}
		}
		else
		{
			BlackboardComponent->SetValueAsBool(BlackboardKeys->RunIndefinitelyBBKey.SelectedKeyName, true);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName, false);
		}
	}
	else
	{
		bool bGoalRequested = RequestNextNpcGoal();
		if (!bGoalRequested)
			return false;
	}
	
	bGoalAssessed = true;

	if (SquadId.IsValid() && bSquadLeader)
		GetWorld()->GetSubsystem<UNpcActivitySquadSubsystem>()->SetGoalForSquadMembers(SquadId, ActiveNpcGoal.NpcGoal->SubordinateGoalChain);

	return true;
}

bool UNpcActivityComponent::RequestNextNpcGoal()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcActivityComponent::RequestNextNpcGoal)
	
	if (CurrentActivityOriginType == EActivityOriginType::Subordinate)
	{
		UE_VLOG(GetOwner(), LogNpcActivity, Warning, TEXT("Attempt to request a new goal when acting as a subordinate. Perhaps request squad leader for next goal?"));
		return false;
		// TODO request next goal from leader○
	}
	else
	{
		if (!ensure(CurrentActivity)) // this might happen, if an activity is not set for time of day, for example
			return false;

		// TODO - different next goal picks: random, weighted, evaluated(?)
		if (CurrentActivity->NpcGoalsChains.Num() == 0)
		{
			UE_VLOG(GetOwner(), LogNpcActivity, Warning, TEXT("Current activity doesn't have goal chains"));
			return false;
		}
		
		if (NpcGoalIndex + 1 >= ActiveNpcGoalChain.NpcGoals.Num() || ActiveNpcGoalChain.NpcGoals.Num() == 0 || ActiveNpcGoalChainIndex == -1)
		{
			// Start new goal chain
			ActiveNpcGoalChainIndex = CurrentActivity->bPickRandomGoalChain && CurrentActivity->NpcGoalsChains.Num() > 2
				? FMath::RandRange(0, CurrentActivity->NpcGoalsChains.Num() - 1)
				: (ActiveNpcGoalChainIndex + 1) % CurrentActivity->NpcGoalsChains.Num();

			auto NpcActivitySquadSubsystem = GetWorld()->GetSubsystem<UNpcActivitySquadSubsystem>();
			if (ActiveNpcGoalChain.DesiredFollowers > 0 && SquadId.IsValid() && bSquadLeader)
				NpcActivitySquadSubsystem->DisbandSquad(this);
				
			ActiveNpcGoalChain = CurrentActivity->NpcGoalsChains[ActiveNpcGoalChainIndex];
			if (ActiveNpcGoalChain.DesiredFollowers > 0)
			{
				NpcActivitySquadSubsystem->CreateSquad(this, ActiveNpcGoalChain.SuitableSquadMembersIds, ActiveNpcGoalChain.SuitableSquadMembersFilter,
					5000.f, ActiveNpcGoalChain.DesiredFollowers, ActiveNpcGoalChain.SquadMemberAttitudePreset);
			}
			
			// NpcComponent->SetAttitudePreset(ActiveNpcGoalChain.AttitudePreset);
			// NpcComponent->SetPerceptionReactionEvaluatorPreset(ActiveNpcGoalChain.ReactionEvaluatorPreset);
			NpcGoalIndex = 0;
			UE_VLOG(GetOwner(), LogNpcActivity, Verbose, TEXT("Started new npc goal chain [%d] %s"), ActiveNpcGoalChainIndex, *ActiveNpcGoalChain.Description);
		}
		else
		{
			NpcGoalIndex++;
			UE_VLOG(GetOwner(), LogNpcActivity, Verbose, TEXT("Starting next NPC goal [%d] %s in activity chain [%d] %s"),
				NpcGoalIndex, *ActiveNpcGoalChain.NpcGoals[NpcGoalIndex].GetDescription(), ActiveNpcGoalChainIndex, *ActiveNpcGoalChain.Description);
		}

		if (IsValid(ActiveNpcGoal.NpcGoal))
		{
			if (CurrentActivityOriginType == EActivityOriginType::Quest)
				NpcGoalCompletedEvent.Broadcast(ActiveNpcGoal.NpcGoal->GoalId, ActiveNpcGoal.NpcGoal->CustomGoalTags);
			
			ActiveNpcGoal.NpcGoal->EndGoal(this);
		}
		
		if (ensure(NpcGoalIndex < ActiveNpcGoalChain.NpcGoals.Num()))
			ActiveNpcGoal = ActiveNpcGoalChain.NpcGoals[NpcGoalIndex];
	}
	
	bGoalAssessed = false;
	return true;
}

void UNpcActivityComponent::SuspendActiveGoal()
{
	if (IsValid(ActiveNpcGoal.NpcGoal))
		ActiveNpcGoal.NpcGoal->SuspendGoal(this);
}

void UNpcActivityComponent::SetSubordinateNpcGoal(const FNpcGoalChain& SubordinateGoalChain)
{
	ActiveNpcGoalChain = SubordinateGoalChain;
	ActiveNpcGoal = SubordinateGoalChain.NpcGoals[0];
	CurrentActivityOriginType = EActivityOriginType::Subordinate;

	bGoalAssessed = false;
	auto BlackboardKeys = NpcComponent->GetNpcDTR()->NpcBlackboardDataAsset;
	BlackboardComponent->SetValueAsBool(BlackboardKeys->RequestResetGoalBBKey.SelectedKeyName, true);
}

void UNpcActivityComponent::ResetSubordinateGoal()
{
	CurrentActivityOriginType = EActivityOriginType::Lifecycle;
	bGoalAssessed = false;
	RunActivity(NpcComponent->GetNpcDTR(), NpcController->GetDayTime());
}

void UNpcActivityComponent::LeaveSquad()
{
	SquadId.Invalidate();
	const FGameplayTag& CurrentDayTime = NpcController->GetDayTime();
	RunActivity(NpcComponent->GetNpcDTR(), CurrentDayTime);
}

void UNpcActivityComponent::SetActivityLocation(const FGameplayTag& ActivityLocationIdTag)
{
	CurrentActivityLocationIdTag = ActivityLocationIdTag;
}

const UNpcGoalBase* UNpcActivityComponent::GetActiveGoal() const
{
	return ActiveNpcGoal.NpcGoal;
}

APawn* UNpcActivityComponent::GetNpcPawn() const
{
	return Cast<AAIController>(GetOwner())->GetPawn();
}

bool UNpcActivityComponent::IsAtLocation(const FGameplayTag& LocationId) const
{
	return Npc->IsAtLocation(LocationId);
}

ENpcGoalAdvanceResult UNpcActivityComponent::AdvanceCurrentGoal(bool bCurrentGoalStepResult, const FGameplayTagContainer& GoalExecutionResultTags)
{
	auto NpcDTR = NpcComponent->GetNpcDTR();
	if (IsValid(ActiveNpcGoal.NpcGoal))
	{
		return ActiveNpcGoal.NpcGoal->AdvanceGoal(BlackboardComponent.Get(), NpcDTR->NpcBlackboardDataAsset,
			this,  bCurrentGoalStepResult, GoalExecutionResultTags);
	}

	return ENpcGoalAdvanceResult::Failed;
}

void UNpcActivityComponent::OnNpcQueueMemberAdvanced(AActor* NpcActor, const FNpcQueueMemberPosition& NpcQueueMemberPosition)
{
	if (NpcActor == GetNpcPawn() && ActiveNpcGoal.NpcGoal && ActiveNpcGoal.NpcGoal->NpcGoalType == ENpcGoalType::StayInQueue)
	{
		auto NpcGoalStayInQueue = Cast<UNpcGoalStayInQueue>(ActiveNpcGoal.NpcGoal);
		auto NpcDTR = NpcComponent->GetNpcDTR();
		NpcGoalStayInQueue->UpdateQueuePosition(BlackboardComponent.Get(), NpcDTR->NpcBlackboardDataAsset, NpcQueueMemberPosition);
	}
}

uint8* UNpcActivityComponent::AllocateGoalMemory(const FGuid& GoalId, size_t Size)
{
	if (!ensure(!GoalsMemories.Contains(GoalId)))
	{
		UE_VLOG(GetOwner(), LogARPGAI, Warning, TEXT("Trying to allocate goal memory for %s but the NpcActivityComponent already has memory for this goal allocated"), *GoalId.ToString());
		return *GoalsMemories.Find(GoalId);
	}

	uint8* NewGoalMemory = new uint8[Size];
	memset(NewGoalMemory, 0, Size);
	GoalsMemories.Add(GoalId, NewGoalMemory);
	return NewGoalMemory;
}

uint8* UNpcActivityComponent::GetGoalMemory(const FGuid& GoalId)
{
	uint8** GoalMemoryPtr = GoalsMemories.Find(GoalId);
	return GoalMemoryPtr ? *GoalMemoryPtr : nullptr;		
}

void UNpcActivityComponent::ClearGoalMemory(const FGuid& GoalId)
{
	if (uint8** GoalMemoryPtr = GoalsMemories.Find(GoalId))
	{
		uint8* GoalMemory = *GoalMemoryPtr;
		delete GoalMemory;
		GoalsMemories.Remove(GoalId);
	}
}

void UNpcActivityComponent::ExternalCompleteGoal()
{
	auto NpcDTR = NpcComponent->GetNpcDTR();
	if (NpcDTR->NpcBlackboardDataAsset->ExternalCompleteGoalBBKey.SelectedKeyName.IsValid())
	{
		BlackboardComponent->SetValueAsBool(NpcDTR->NpcBlackboardDataAsset->ExternalCompleteGoalBBKey.SelectedKeyName, true);
	}
}

void UNpcActivityComponent::SetRemainingGoalExecutuionTime(const float InTimeLeft)
{
	RemainingGoalExecutionTime = InTimeLeft;
}