#include "Activities/NpcGoals.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BrainComponent.h"
#include "Activities/ActivityInstancesHelper.h"
#include "Activities/NpcGoalBackgroundTasks.h"
#include "GameFramework/GameModeBase.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Components/NpcQueueComponent.h"
#include "Components/Controller/NpcActivityComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GameFramework/Character.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/NpcActivitySquadSubsystem.h"
#include "Subsystems/NpcPatrolRoutesSubsystem.h"
#include "Subsystems/NpcQueueSubsystem.h"
#include "Subsystems/NpcRegistrationSubsystem.h"

#pragma region UNpcGoalBase

UNpcGoalBase::UNpcGoalBase()
{
	GoalId = FGuid::NewGuid();
}

const FGuid& UNpcGoalBase::GetGoalId() const
{
	return GoalId;
}

ENpcGoalStartResult UNpcGoalBase::Start(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
	UNpcActivityComponent* NpcActivityComponent) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcGoalBase::Start)
	
	BlackboardComponent->ClearValue(BlackboardKeys->GoalExecutionTimeLeftBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->RunIndefinitelyBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->GoalTypeBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->EqsToRunBBKey.SelectedKeyName);

	BlackboardComponent->ClearValue(BlackboardKeys->LocationToGoBBKey.SelectedKeyName);
	BlackboardComponent->ClearValue(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName);

	BlackboardComponent->SetValueAsEnum(BlackboardKeys->GoalTypeBBKey.SelectedKeyName, (uint8)NpcGoalType);

	Restore(BlackboardComponent, BlackboardKeys, NpcActivityComponent, true);
	
	return ENpcGoalStartResult::InProgress;
}

ENpcGoalStartResult UNpcGoalBase::Restore(UBlackboardComponent* BlackboardComponent,
                                          const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent, bool bInitialStart) const
{
	if (CharacterStateTag.IsValid())
		BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->NpcActivityStateBBKey.SelectedKeyName, CharacterStateTag.GetSingleTagContainer());

	if (auto Npc = Cast<INpc>(NpcActivityComponent->GetNpcPawn()))
		Npc->GiveNpcTags(GrantedTagsDuringGoal);

	auto NpcComponent = NpcActivityComponent->GetNpcPawn()->FindComponentByClass<UNpcComponent>();
	NpcComponent->SetAttitudePreset(AttitudePreset);

	for (auto* BackgroundTask : BackgroundTasks)
		BackgroundTask->Start(NpcActivityComponent, bInitialStart);

	if (!BlackboardKeys->GoalTagsBBKey.SelectedKeyName.IsNone())
		BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GoalTagsBBKey.SelectedKeyName, CustomGoalTags);
	
	return ENpcGoalStartResult::InProgress;
}

ENpcGoalAdvanceResult UNpcGoalUseSmartObject::AdvanceGoal(UBlackboardComponent* BlackboardComponent,
	const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent,
	bool bCurrentPhaseResult, const FGameplayTagContainer& GoalExecutionResultTags)
{
	if (bCurrentPhaseResult && bRepeatUntilNoInteractableActorsLeft)
	{
		return GoalExecutionResultTags.HasTag(AIGameplayTags::AI_Activity_Goal_Result_SmartObject_NotFound)
			? ENpcGoalAdvanceResult::Completed : ENpcGoalAdvanceResult::InProgress; 
	}
	else
	{
		return Super::AdvanceGoal(BlackboardComponent, BlackboardKeys, NpcActivityComponent, bCurrentPhaseResult,
			GoalExecutionResultTags);	
	}
}

FString UNpcGoalUseSmartObject::GetDescription() const
{
	// TODO add details about tag filters
	return FString::Printf(TEXT("Use Smart Object\n"));
}

FString UNpcGoalBase::GetDescription() const
{
	return GetClass()->GetName();
}

ENpcGoalAdvanceResult UNpcGoalBase::AdvanceGoal(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
                                                UNpcActivityComponent* NpcActivityComponent, bool bCurrentPhaseResult, const FGameplayTagContainer&
                                                GoalExecutionResultTags)
{
	BlackboardComponent->SetValueAsEnum(BlackboardKeys->GoalTypeBBKey.SelectedKeyName, (uint8)NpcGoalType);
	return bCurrentPhaseResult ? ENpcGoalAdvanceResult::Completed : ENpcGoalAdvanceResult::Failed;
}

void UNpcGoalBase::SuspendGoal(UNpcActivityComponent* NpcActivityComponent)
{
	if (auto Npc = Cast<INpc>(NpcActivityComponent->GetNpcPawn()))
		Npc->RemoveNpcTags(GrantedTagsDuringGoal);

	for (auto* BackgroundTask : BackgroundTasks)
		BackgroundTask->Stop(NpcActivityComponent);
}

void UNpcGoalBase::EndGoal(UNpcActivityComponent* NpcActivityComponent)
{
	SuspendGoal(NpcActivityComponent);
	NpcActivityComponent->ClearGoalMemory(GoalId);
}

#pragma endregion UNpcGoalBase

#pragma region UNpcGoalVisitLocation

ENpcGoalStartResult UNpcGoalVisitLocation::Start(UBlackboardComponent* BlackboardComponent,
                                                 const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent*
                                                 NpcActivityComponent) const
{
	auto Result = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	if (bCompleteOnEntering)
	{
		if (NpcActivityComponent->IsAtLocation(LocationIdTag))
		{
			return ENpcGoalStartResult::Finished;
		}
	}
	
	auto NpcGameMode = Cast<INpcSystemGameMode>(BlackboardComponent->GetWorld()->GetAuthGameMode());
	if (!ensure(NpcGameMode))
		return ENpcGoalStartResult::Failed;
	
	FVector Location = NpcGameMode->GetNpcLocation(LocationIdTag, true);
	if (Location != FAISystem::InvalidLocation)
		BlackboardComponent->SetValueAsVector(BlackboardKeys->LocationToGoBBKey.SelectedKeyName, Location);

	NpcActivityComponent->SetActivityLocation(LocationIdTag);
	return Result;
}

#pragma endregion UNpcGoalVisitLocation

#pragma region UNpcGoalGesture

ENpcGoalStartResult UNpcGoalGesture::Start(UBlackboardComponent* BlackboardComponent,
                                           const UNpcBlackboardDataAsset* BlackboardKeys,
                                           UNpcActivityComponent* NpcActivityComponent) const
{
	auto Result = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName, GestureTag.GetSingleTagContainer());
	return Result;
}

ENpcGoalStartResult UNpcGoalWander::Restore(UBlackboardComponent* BlackboardComponent,
	const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent,
	bool bInitialStart) const
{
	auto RestoreResult = Super::Restore(BlackboardComponent, BlackboardKeys, NpcActivityComponent, bInitialStart);
	if (RestoreResult == ENpcGoalStartResult::Failed)
		return RestoreResult;

	BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, AreaEqs);
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName, GestureOptionsTags);
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->ActivityPhrasesBBKey.SelectedKeyName, SpeechOptionsTags);

	return ENpcGoalStartResult::InProgress;
}

#pragma endregion UNpcGoalGesture

#pragma region UNpcGoalWander


#pragma endregion UNpcGoalWander

#pragma region UNpcGoalFollowLeader

ENpcGoalStartResult UNpcGoalFollowLeader::Start(UBlackboardComponent* BlackboardComponent,
                                                const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent*
                                                NpcActivityComponent) const
{
	auto SquadId = NpcActivityComponent->GetSquadId();
	if (!ensure(SquadId.IsValid()))
		return ENpcGoalStartResult::Failed;
	
	auto Result = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	switch (NpcSquadFollowType)
	{
		case ENpcSquadFollowType::Around:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, 0.f);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, 0.f);
		break;
		case ENpcSquadFollowType::InFront:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, DotProductScore);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, 0.85f);
			break;
		case ENpcSquadFollowType::NextTo:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, DotProductScore);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, 0.0f);
			break;
		case ENpcSquadFollowType::Behind:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, DotProductScore);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, -0.85f);
			break;
		case ENpcSquadFollowType::NotInFrontOf:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, -DotProductScore);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, 0.85);
			break;
		default:
			ensure(false);
		break;
	}

	auto NpcActivitySquadComponent = UNpcActivitySquadSubsystem::Get(NpcActivityComponent);
	auto SquadLeader = NpcActivitySquadComponent->GetSquadLeader(SquadId);
	BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderCircleRadiusBBKey.SelectedKeyName, FollowRadius);
	BlackboardComponent->SetValueAsObject(BlackboardKeys->FollowTargetBBKey.SelectedKeyName, SquadLeader->GetNpcPawn());
	
	return Result;
}

#pragma endregion UNpcGoalFollowLeader

#pragma region UNpcGoalFindArea

ENpcGoalStartResult UNpcGoalFindArea::Start(UBlackboardComponent* BlackboardComponent,
                                            const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent*
                                            NpcActivityComponent) const
{
	auto Result = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, AreaEqs);
	return Result;
	// you can put some EQS parameters into the NpcActivityComponent here 
}

#pragma endregion UNpcGoalFindArea

#pragma region UNpcGoalConversate

bool UNpcGoalConversate::SetConversationBlackboardContext(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent) const
{
	if (bUseEQS)
	{
		BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, ConversationPartnersEQS);
		return true;
	}
	else if (ConversationPartnerId.IsValid())
	{
		auto NpcRegistrationSubsystem = UNpcRegistrationSubsystem::Get(NpcActivityComponent);
		auto ConversationPartner = NpcRegistrationSubsystem->GetClosestNpc(ConversationPartnerId, NpcActivityComponent->GetPawnLocation(), &ConversationPartnerTagsFilter);
		if (ConversationPartner)
		{
			BlackboardComponent->SetValueAsObject(BlackboardKeys->ConversationPartnerBBKey.SelectedKeyName, ConversationPartner->GetOwner());
			return true;
		}
	}
	return false;
}

ENpcGoalStartResult UNpcGoalConversate::Start(UBlackboardComponent* BlackboardComponent,
                                              const UNpcBlackboardDataAsset* BlackboardKeys,
                                              UNpcActivityComponent* NpcActivityComponent) const
{
	auto Result = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	if (Result != ENpcGoalStartResult::InProgress)
		return Result;
	
	SetConversationBlackboardContext(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	return ENpcGoalStartResult::Failed;
}

ENpcGoalStartResult UNpcGoalConversate::Restore(UBlackboardComponent* BlackboardComponent,
	const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent,
	bool bInitialStart) const
{
	Super::Restore(BlackboardComponent, BlackboardKeys, NpcActivityComponent, bInitialStart);
	SetConversationBlackboardContext(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	return ENpcGoalStartResult::InProgress;
}

#pragma endregion UNpcGoalConversate

#pragma region UNpcGoalTalkToPlayer

ENpcGoalStartResult UNpcGoalTalkToPlayer::Start(UBlackboardComponent* BlackboardComponent,
	const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent) const
{
	auto Result = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	if (bGoToPlayerDirectly)
	{
		auto PlayerCharacter = UGameplayStatics::GetPlayerCharacter(NpcActivityComponent, 0);
		BlackboardComponent->SetValueAsObject(BlackboardKeys->ConversationPartnerBBKey.SelectedKeyName, PlayerCharacter);
		BlackboardComponent->ClearValue(BlackboardKeys->EqsToRunBBKey.SelectedKeyName);
	}
	else
	{
		BlackboardComponent->ClearValue(BlackboardKeys->ConversationPartnerBBKey.SelectedKeyName);
		BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, PlayerSearchEQS);
	}
	
	return Result;
}

#pragma endregion UNpcGoalTalkToPlayer

#pragma region UNpcGoalUseSmartObject

ENpcGoalStartResult UNpcGoalUseSmartObject::Start(UBlackboardComponent* BlackboardComponent,
                                                  const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent*
                                                  NpcActivityComponent) const
{
	auto Result = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	auto SmartObjectEqsSoftPtr = GetDefault<UNpcSettings>()->SmartObjectNpcGoalEqs;
	if (ensure(!SmartObjectEqsSoftPtr.IsNull()))
	{
		auto SmartObjectEqs = SmartObjectEqsSoftPtr.LoadSynchronous();
		if (IsValid(SmartObjectEqs))
			BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, SmartObjectEqs);
	}
	
	BlackboardComponent->ClearValue(BlackboardKeys->InteractionActorBBKey.SelectedKeyName);
	return Result;
}

FNpcGoalParameters_UseSmartObject UNpcGoalUseSmartObject::GetParameters(const APawn* GoalExecutor) const
{
	if (NpcGoalDataParameterId.IsValid())
	{
		if (auto NpcComponent = GoalExecutor->FindComponentByClass<UNpcComponent>())
		{
			const FNpcGoalParameters_UseSmartObject* Result = NpcComponent->GetNpcGoalParameters<FNpcGoalParameters_UseSmartObject>(NpcGoalDataParameterId);
			return *Result;
		}
	}

	FNpcGoalParameters_UseSmartObject DefaultParameters;
	DefaultParameters.LocationIdTag = LocationIdTag;
	DefaultParameters.LocationSearchRadius = LocationSearchRadius;
	DefaultParameters.IntentionFilter = IntentionFilter;
	DefaultParameters.SmartObjectActorFilter = SmartObjectActorFilter;
	return DefaultParameters;
}

#pragma endregion UNpcGoalUseSmartObject

#pragma region UNpcGoalPatrol

ENpcGoalStartResult UNpcGoalPatrol::Start(UBlackboardComponent* BlackboardComponent,
                                          const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent) const
{
	auto Result = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	auto NpcPatrolRouteSubsystem = UNpcPatrolRoutesSubsystem::Get(NpcActivityComponent);
	auto Parameters = GetParameters(NpcActivityComponent->GetNpcPawn());
	
	if (!ensure(NpcPatrolRouteSubsystem) || !ensure(Parameters.PatrolRouteId.IsValid()))
		return ENpcGoalStartResult::Failed;

	auto PatrolRoute = NpcPatrolRouteSubsystem->StartPatrolRoute(NpcActivityComponent->GetNpcPawn(), Parameters.PatrolRouteId, bPreferClosestRoute, bUsePathfinding);
	if (!PatrolRoute.IsValid())
		return ENpcGoalStartResult::Failed;

	BlackboardComponent->SetValueAsVector(BlackboardKeys->LocationToGoBBKey.SelectedKeyName, PatrolRoute.RoutePointLocation);

	UpdateStayAtPatrolPointTime(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	
	return Result;
}

ENpcGoalStartResult UNpcGoalPatrol::Restore(UBlackboardComponent* BlackboardComponent,
                                            const UNpcBlackboardDataAsset* BlackboardKeys,
                                            UNpcActivityComponent* NpcActivityComponent, bool bInitialStart) const
{
	ENpcGoalStartResult RestoreState = Super::Restore(BlackboardComponent, BlackboardKeys, NpcActivityComponent, bInitialStart);
	if (RestoreState == ENpcGoalStartResult::Failed)
		return RestoreState;
	
	auto NpcPatrolRouteSubsystem = UNpcPatrolRoutesSubsystem::Get(NpcActivityComponent);
	auto NpcRouteData = NpcPatrolRouteSubsystem->GetActivePatrolRoute(NpcActivityComponent->GetNpcPawn());
	if (NpcRouteData.IsValid())
	{
		BlackboardComponent->SetValueAsVector(BlackboardKeys->LocationToGoBBKey.SelectedKeyName, NpcRouteData.RoutePointLocation);
		UpdateStayAtPatrolPointTime(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	}

	return NpcRouteData.IsValid() ? ENpcGoalStartResult::InProgress : ENpcGoalStartResult::Finished;
}

ENpcGoalAdvanceResult UNpcGoalPatrol::AdvanceGoal(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent,
                                                  bool bCurrentPhaseResult, const FGameplayTagContainer& GoalExecutionResultTags)
{
	ENpcGoalAdvanceResult Result = Super::AdvanceGoal(BlackboardComponent, BlackboardKeys, NpcActivityComponent, bCurrentPhaseResult, GoalExecutionResultTags);
	if (Result == ENpcGoalAdvanceResult::Failed)
		return Result;
	
	auto NpcPatrolRouteSubsystem = UNpcPatrolRoutesSubsystem::Get(NpcActivityComponent);
	FNpcPatrolRouteAdvanceResult NpcRouteAdvanceResult = NpcPatrolRouteSubsystem->GetNextPatrolRoutePoint(NpcActivityComponent->GetNpcPawn());
	if (NpcRouteAdvanceResult.LoopCount >= Loops)
		return ENpcGoalAdvanceResult::Completed;

	BlackboardComponent->SetValueAsVector(BlackboardKeys->LocationToGoBBKey.SelectedKeyName, NpcRouteAdvanceResult.NextLocation);
	UpdateStayAtPatrolPointTime(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	return ENpcGoalAdvanceResult::InProgress;
}

void UNpcGoalPatrol::EndGoal(UNpcActivityComponent* NpcActivityComponent)
{
	auto NpcPatrolRouteSubsystem = UNpcPatrolRoutesSubsystem::Get(NpcActivityComponent);
	NpcPatrolRouteSubsystem->StopPatrolRoute(NpcActivityComponent->GetNpcPawn());
	
	Super::EndGoal(NpcActivityComponent);
}

void UNpcGoalPatrol::UpdateStayAtPatrolPointTime(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent) const
{
	if (auto NpcGameMode = Cast<INpcSystemGameMode>(NpcActivityComponent->GetWorld()->GetAuthGameMode()))
	{
		float GameTimeToRealTimeCoefficient = 3600.f / NpcGameMode->GetTimeRateSeconds();
		const float WaitTime = FMath::RandRange(StayAtEachPatrolPointTimeMin * GameTimeToRealTimeCoefficient, StayAtEachPatrolPointTimeMax * GameTimeToRealTimeCoefficient);
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->StayAtPatrolPointTimeBBKey.SelectedKeyName, WaitTime);
	}
}

FNpcGoalParameters_Patrol UNpcGoalPatrol::GetParameters(const APawn* GoalExecutor) const
{
	if (NpcGoalDataParameterId.IsValid())
	{
		if (auto NpcComponent = GoalExecutor->FindComponentByClass<UNpcComponent>())
		{
			const FNpcGoalParameters_Patrol* Result = NpcComponent->GetNpcGoalParameters<FNpcGoalParameters_Patrol>(NpcGoalDataParameterId);
			return *Result;
		}
	}

	FNpcGoalParameters_Patrol DefaultParameters;
	DefaultParameters.PatrolRouteId = PatrolRouteId;
	return DefaultParameters;
}

#pragma endregion UNpcGoalPatrol

#pragma region UNpcGoalStayInQueue

ENpcGoalStartResult UNpcGoalStayInQueue::Start(UBlackboardComponent* BlackboardComponent,
	const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent) const
{
	auto GoalStartResult = Super::Start(BlackboardComponent, BlackboardKeys, NpcActivityComponent);
	if (GoalStartResult != ENpcGoalStartResult::InProgress)
		return GoalStartResult;

	auto NpcQueueSubsystem = NpcActivityComponent->GetWorld()->GetSubsystem<UNpcQueueSubsystem>();
	if (!ensure(NpcQueueSubsystem))
		return ENpcGoalStartResult::Failed;

	auto NpcQueueComponent = NpcQueueSubsystem->GetQueue(QueueId);
	if (!NpcQueueComponent)
	{
		UE_VLOG(NpcActivityComponent->GetOwner(), LogARPGAI, Warning, TEXT("UNpcGoalStayInQueue::Start: Failed to find queue %s"), *QueueId.ToString());
		return ENpcGoalStartResult::Failed;
	}

	if (NpcQueueComponent->IsFull())
	{
		UE_VLOG(NpcActivityComponent->GetOwner(), LogARPGAI, Warning, TEXT("UNpcGoalStayInQueue::Start: Queue is full, consider adding more queue points %s"), *QueueId.ToString());
		return ENpcGoalStartResult::Failed;
	}

	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GoalStateTagBBKey.SelectedKeyName,
		AIGameplayTags::Activity_Goal_State_StayInQueue_Enter.GetTag().GetSingleTagContainer());
	BlackboardComponent->SetValueAsObject(BlackboardKeys->InteractionActorBBKey.SelectedKeyName, NpcQueueComponent->GetOwner());
	
	return ENpcGoalStartResult::InProgress;
}

ENpcGoalAdvanceResult UNpcGoalStayInQueue::AdvanceGoal(UBlackboardComponent* BlackboardComponent,
	const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent,
	bool bCurrentPhaseResult, const FGameplayTagContainer& GoalExecutionResultTags)
{
	auto NpcGoalResult = Super::AdvanceGoal(BlackboardComponent, BlackboardKeys, NpcActivityComponent, bCurrentPhaseResult, GoalExecutionResultTags);
	if (NpcGoalResult == ENpcGoalAdvanceResult::Failed)
		return NpcGoalResult;

	auto NpcQueueComponent = NpcActivityComponent->GetWorld()->GetSubsystem<UNpcQueueSubsystem>()->GetQueue(QueueId);
	if (!NpcQueueComponent)
		return ENpcGoalAdvanceResult::Failed;
	
	if (GoalExecutionResultTags.First() == AIGameplayTags::Activity_Goal_State_StayInQueue_Enter)
	{
		// 1. request place in queue
		auto NpcQueueMemberPosition = NpcQueueComponent->StandInQueue(NpcActivityComponent->GetNpcPawn());
		if (!NpcQueueMemberPosition.bEntered)
		{
			UE_VLOG(NpcActivityComponent->GetOwner(), LogARPGAI, Warning, TEXT("UNpcGoalStayInQueue::AdvanceGoal: Fail, not entered queue %s"), *QueueId.ToString());
			return ENpcGoalAdvanceResult::Failed;
		}

		// 2. prepare blackboard 
		// 2.1. reset goal state, set queue position location and rotation
		// 2.2. if first place in queue - set waiting in queue time, if applicable
		UpdateQueuePosition(BlackboardComponent, BlackboardKeys, NpcQueueMemberPosition);

		// 2.3. Set optional gesture tag to be used in queue 
		if (QueueGestureTag.IsValid())
			BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName, QueueGestureTag.GetSingleTagContainer());
		else
			BlackboardComponent->ClearValue(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName);
		
		// 3. Set optional tag for standing in queue. Usage suggestion: change idle posture 
		if (!OptionalTagWhenInQueue.IsEmpty())
		{
			const auto& OptionalTags = OptionalTagWhenInQueue.GetGameplayTagArray();
			const FGameplayTag& NewStateTag = OptionalTags.Num() > 1 ? OptionalTags[FMath::RandRange(0, OptionalTags.Num() - 1)] : OptionalTags[0];
			auto Npc = Cast<INpc>(NpcActivityComponent->GetNpcPawn());
			Npc->GiveNpcTags(NewStateTag.GetSingleTagContainer());

			auto GoalMemory = reinterpret_cast<FNpcGoalMemory_StayInQueue*>(NpcActivityComponent->AllocateGoalMemory(GoalId, sizeof(FNpcGoalMemory_StayInQueue)));
			GoalMemory->AppliedOptionalTag = NewStateTag;
		
		}

		// 4.Subscribe to queue's event when queue advances
		NpcQueueComponent->NpcQueueMemberAdvancedEvent.AddUObject(NpcActivityComponent, &UNpcActivityComponent::OnNpcQueueMemberAdvanced);
		
		return ENpcGoalAdvanceResult::InProgress;
	}
	else if(GoalExecutionResultTags.First() == AIGameplayTags::Activity_Goal_State_StayInQueue_Finished)
	{
		// 18.12.2024 @AK: this might be redundant, since ::SuspendGoal should be called always when the goal is completed
		// NpcQueueComponent->LeaveQueue(NpcActivityComponent->GetNpcPawn());
		return ENpcGoalAdvanceResult::Completed;
	}

	ensure(false); // idk, wtf?
	return ENpcGoalAdvanceResult::Completed;
}

ENpcGoalStartResult UNpcGoalStayInQueue::Restore(UBlackboardComponent* BlackboardComponent,
                                                 const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent, bool bInitialStart) const
{
	auto RestoreResult = Super::Restore(BlackboardComponent, BlackboardKeys, NpcActivityComponent, bInitialStart);
	if (RestoreResult != ENpcGoalStartResult::InProgress)
		return RestoreResult;

	auto NpcQueueSubsystem = NpcActivityComponent->GetWorld()->GetSubsystem<UNpcQueueSubsystem>();
	if (auto NpcQueueComponent = NpcQueueSubsystem->GetQueue(QueueId))
	{
		if (!NpcQueueComponent->NpcQueueMemberAdvancedEvent.IsBoundToObject(NpcActivityComponent))
			NpcQueueComponent->NpcQueueMemberAdvancedEvent.AddUObject(NpcActivityComponent, &UNpcActivityComponent::OnNpcQueueMemberAdvanced);
		
		const FNpcQueueMemberPosition& NpcQueueMemberPosition = NpcQueueComponent->GetNpcQueuePosition(NpcActivityComponent->GetNpcPawn());
		if (!NpcQueueMemberPosition.bEntered)
			return ENpcGoalStartResult::Failed;
		
		UpdateQueuePosition(BlackboardComponent, BlackboardKeys, NpcQueueMemberPosition);

		auto GoalMemory = reinterpret_cast<FNpcGoalMemory_StayInQueue*>(NpcActivityComponent->GetGoalMemory(GoalId));
		if (GoalMemory != nullptr)
		{
			auto Npc = Cast<INpc>(NpcActivityComponent->GetNpcPawn());
			Npc->GiveNpcTags(GoalMemory->AppliedOptionalTag.GetSingleTagContainer()); 
		}
	}

	return ENpcGoalStartResult::InProgress;
}

void UNpcGoalStayInQueue::SuspendGoal(UNpcActivityComponent* NpcActivityComponent)
{
	Super::SuspendGoal(NpcActivityComponent);
	auto NpcQueueSubsystem = NpcActivityComponent->GetWorld()->GetSubsystem<UNpcQueueSubsystem>();
	if (auto NpcQueueComponent = NpcQueueSubsystem->GetQueue(QueueId))
	{
		NpcQueueComponent->NpcQueueMemberAdvancedEvent.RemoveAll(NpcActivityComponent);
		NpcQueueComponent->LeaveQueue(NpcActivityComponent->GetNpcPawn());
	}
	
	auto NpcGoalMemory = reinterpret_cast<FNpcGoalMemory_StayInQueue*>(NpcActivityComponent->GetGoalMemory(GoalId));
	if (NpcGoalMemory != nullptr && NpcGoalMemory->AppliedOptionalTag.IsValid())
	{
		auto Npc = Cast<INpc>(NpcActivityComponent->GetNpcPawn());
		Npc->RemoveNpcTags(NpcGoalMemory->AppliedOptionalTag.GetSingleTagContainer());
	}
}

void UNpcGoalStayInQueue::UpdateQueuePosition(UBlackboardComponent* BlackboardComponent,
	const UNpcBlackboardDataAsset* BlackboardKeys, const FNpcQueueMemberPosition& NpcQueueMemberPosition) const
{
	BlackboardComponent->ClearValue(BlackboardKeys->GoalStateTagBBKey.SelectedKeyName);
	BlackboardComponent->SetValueAsVector(BlackboardKeys->QueuePointLocationBBKey.SelectedKeyName, NpcQueueMemberPosition.QueuePointLocation);
	BlackboardComponent->SetValueAsRotator(BlackboardKeys->QueuePointRotationBBKey.SelectedKeyName, NpcQueueMemberPosition.QueuePointRotation);

	UE_VLOG_LOCATION(BlackboardComponent->GetOwner(), LogARPGAI, Verbose, NpcQueueMemberPosition.QueuePointLocation, 12.5f, FColor::Cyan, TEXT("Queue location"));
	
	if (NpcQueueMemberPosition.PlaceInQueue == 0 && !bStayInQueueIndefinitely)
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->StayAtTheBeginningOfQueueTimeBBKey.SelectedKeyName, FirstInQueueGameTimeDuration);
	else
		BlackboardComponent->ClearValue(BlackboardKeys->StayAtTheBeginningOfQueueTimeBBKey.SelectedKeyName);
}

#pragma endregion UNpcGoalStayInQueue
