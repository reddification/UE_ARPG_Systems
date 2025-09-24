// 


#include "FlowGraph/Nodes/FlowNode_NpcGoal_StayInQueue.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Components/NpcQueueComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Subsystems/NpcQueueSubsystem.h"

ENpcGoalStartResult UFlowNode_NpcGoal_StayInQueue::Start()
{
	auto GoalStartResult = Super::Start();
	if (GoalStartResult != ENpcGoalStartResult::InProgress)
		return GoalStartResult;

	auto NpcQueueSubsystem = NpcPawn->GetWorld()->GetSubsystem<UNpcQueueSubsystem>();
	if (!ensure(NpcQueueSubsystem))
		return ENpcGoalStartResult::Failed;

	const auto* ActualGoalParameters = GetGoalParameters();
	if (ActualGoalParameters == nullptr)
		return ENpcGoalStartResult::Failed;
	
	auto NpcQueueComponent = NpcQueueSubsystem->GetQueue(ActualGoalParameters->QueueId);
	if (!NpcQueueComponent)
	{
		UE_VLOG(NpcPawn->GetOwner(), LogARPGAI, Warning, TEXT("UNpcGoalStayInQueue::Start: Failed to find queue %s"), *ActualGoalParameters->QueueId.ToString());
		return ENpcGoalStartResult::Failed;
	}

	if (NpcQueueComponent->IsFull())
	{
		UE_VLOG(NpcPawn->GetOwner(), LogARPGAI, Warning, TEXT("UNpcGoalStayInQueue::Start: Queue is full, consider adding more queue points %s"), *ActualGoalParameters->QueueId.ToString());
		return ENpcGoalStartResult::Failed;
	}

	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GoalStateTagBBKey.SelectedKeyName,
		AIGameplayTags::Activity_Goal_State_StayInQueue_Enter.GetTag().GetSingleTagContainer());
	BlackboardComponent->SetValueAsObject(BlackboardKeys->InteractionActorBBKey.SelectedKeyName, NpcQueueComponent->GetOwner());
	
	return ENpcGoalStartResult::InProgress;
}

ENpcGoalAdvanceResult UFlowNode_NpcGoal_StayInQueue::Advance(const FGameplayTagContainer& GoalExecutionResultTags)
{
	auto NpcGoalResult = Super::Advance(GoalExecutionResultTags);
	if (NpcGoalResult == ENpcGoalAdvanceResult::Failed)
		return NpcGoalResult;

	const auto* ActualGoalParameters = GetGoalParameters();
	
	auto NpcQueueComponent = NpcPawn->GetWorld()->GetSubsystem<UNpcQueueSubsystem>()->GetQueue(ActualGoalParameters->QueueId);
	if (!NpcQueueComponent)
		return ENpcGoalAdvanceResult::Failed;
	
	if (GoalExecutionResultTags.HasTagExact(AIGameplayTags::Activity_Goal_State_StayInQueue_Enter))
	{
		// 1. request place in queue
		auto StandInQueueResult = ActualGoalParameters->DesiredQueuePosition >= 0
			? NpcQueueComponent->StandInQueueAtPosition(NpcPawn.Get(), ActualGoalParameters->DesiredQueuePosition)
			: NpcQueueComponent->StandInQueue(NpcPawn.Get());
		
		if (!StandInQueueResult.bEntered)
		{
			UE_VLOG(NpcPawn->GetOwner(), LogARPGAI, Warning, TEXT("UNpcGoalStayInQueue::AdvanceGoal: Fail, not entered queue %s"), *ActualGoalParameters->QueueId.ToString());
			return ENpcGoalAdvanceResult::Failed;
		}

		// 2. prepare blackboard 
		// 2.1. reset goal state, set queue position location and rotation
		// 2.2. if first place in queue - set waiting in queue time, if applicable
		UpdateQueuePosition(StandInQueueResult, ActualGoalParameters);

		// 2.3. Set optional gesture tag to be used in queue 
		if (ActualGoalParameters->OptionalGestureTag.IsValid())
			BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName, ActualGoalParameters->OptionalGestureTag.GetSingleTagContainer());
		else
			BlackboardComponent->ClearValue(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName);
		
		// 3.Subscribe to queue's event when queue advances
		if (ensure(!NpcQueueComponent->NpcQueueMemberAdvancedEvent.IsBoundToObject(this)))
			NpcQueueComponent->NpcQueueMemberAdvancedEvent.AddUObject(this, &UFlowNode_NpcGoal_StayInQueue::OnNpcQueueMemberAdvanced);
		
		return ENpcGoalAdvanceResult::InProgress;
	}
	else if(GoalExecutionResultTags.HasTagExact(AIGameplayTags::Activity_Goal_State_StayInQueue_Finished))
	{
		// 18.12.2024 @AK: this might be redundant, since ::SuspendGoal should be called always when the goal is completed
		// NpcQueueComponent->LeaveQueue(NpcActivityComponent);
		return ENpcGoalAdvanceResult::Completed;
	}

	ensure(false); // idk, wtf?
	return ENpcGoalAdvanceResult::Completed;
}

ENpcGoalStartResult UFlowNode_NpcGoal_StayInQueue::Restore(bool bInitialStart)
{
	auto RestoreResult = Super::Restore(bInitialStart);
	if (RestoreResult != ENpcGoalStartResult::InProgress)
		return RestoreResult;

	auto NpcQueueSubsystem = NpcPawn->GetWorld()->GetSubsystem<UNpcQueueSubsystem>();
	auto ActualGoalParameters = GetGoalParameters();
	if (auto NpcQueueComponent = NpcQueueSubsystem->GetQueue(ActualGoalParameters->QueueId))
	{
		const FNpcQueueMemberPosition& NpcQueueMemberPosition = NpcQueueComponent->GetNpcQueuePosition(NpcPawn.Get());
		if (NpcQueueMemberPosition.bEntered)
		{
			if (!NpcQueueComponent->NpcQueueMemberAdvancedEvent.IsBoundToObject(this))
				NpcQueueComponent->NpcQueueMemberAdvancedEvent.AddUObject(this, &UFlowNode_NpcGoal_StayInQueue::OnNpcQueueMemberAdvanced);
		
			UpdateQueuePosition(NpcQueueMemberPosition, ActualGoalParameters);
		}

		return ENpcGoalStartResult::InProgress;
	}

	return ENpcGoalStartResult::Failed;
}

void UFlowNode_NpcGoal_StayInQueue::Finish()
{
	auto NpcQueueSubsystem = NpcPawn->GetWorld()->GetSubsystem<UNpcQueueSubsystem>();
	auto ActualGoalParameters = GetGoalParameters();
	if (auto NpcQueueComponent = NpcQueueSubsystem->GetQueue(ActualGoalParameters->QueueId))
	{
		NpcQueueComponent->NpcQueueMemberAdvancedEvent.RemoveAll(this);
		NpcQueueComponent->LeaveQueue(NpcPawn.Get());
	}
	
	Super::Finish();
}

void UFlowNode_NpcGoal_StayInQueue::OnNpcQueueMemberAdvanced(AActor* NpcActor,
                                                             const FNpcQueueMemberPosition& NpcQueueMemberPosition)
{
	if (NpcActor != NpcPawn.Get())
		return;
	
	UpdateQueuePosition(NpcQueueMemberPosition, GetGoalParameters());
}

void UFlowNode_NpcGoal_StayInQueue::UpdateQueuePosition(
	const FNpcQueueMemberPosition& NpcQueueMemberPosition,
	const FNpcGoalParameters_StayInQueue* ActualGoalParameters) const
{
	UE_VLOG(NpcPawn.Get(), LogARPGAI, Verbose, TEXT("New queue position: [%d] at [%s]"), NpcQueueMemberPosition.PlaceInQueue, *NpcQueueMemberPosition.QueuePointLocation.ToString());	
	
	BlackboardComponent->ClearValue(BlackboardKeys->GoalStateTagBBKey.SelectedKeyName);
	BlackboardComponent->SetValueAsVector(BlackboardKeys->QueuePointLocationBBKey.SelectedKeyName, NpcQueueMemberPosition.QueuePointLocation);
	BlackboardComponent->SetValueAsRotator(BlackboardKeys->QueuePointRotationBBKey.SelectedKeyName, NpcQueueMemberPosition.QueuePointRotation);

	UE_VLOG_LOCATION(BlackboardComponent->GetOwner(), LogARPGAI, Verbose, NpcQueueMemberPosition.QueuePointLocation, 12.5f, FColor::Cyan, TEXT("Queue location"));

	if (NpcQueueMemberPosition.PlaceInQueue == 0 && !ActualGoalParameters->bStayInQueueIndefinitely)
	{
		auto GameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
		float RealTimeDuration = GameMode->ConvertGameTimeToRealTime(ActualGoalParameters->FirstInQueueGameTimeDuration);
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->StayAtTheBeginningOfQueueTimeBBKey.SelectedKeyName, RealTimeDuration);
	}
	else
	{
		BlackboardComponent->ClearValue(BlackboardKeys->StayAtTheBeginningOfQueueTimeBBKey.SelectedKeyName);
	}
}

void UFlowNode_NpcGoal_StayInQueue::UpdateQueuePosition(const FNpcQueueMemberPosition& NpcQueueMemberPosition) const
{
	auto ActualGoalParameters = GetGoalParameters();
	UpdateQueuePosition(NpcQueueMemberPosition, ActualGoalParameters);
}

const FNpcGoalParameters_StayInQueue* UFlowNode_NpcGoal_StayInQueue::GetGoalParameters() const
{
	const FNpcGoalParameters_StayInQueue* ActualGoalParameters = &NpcGoalParameters;
	if (QueueGoalParametersId.IsValid())
	{
		ActualGoalParameters = NpcComponent->GetNpcGoalParameters<FNpcGoalParameters_StayInQueue>(QueueGoalParametersId);
		ensure(ActualGoalParameters);
	}

	return ActualGoalParameters;
}
