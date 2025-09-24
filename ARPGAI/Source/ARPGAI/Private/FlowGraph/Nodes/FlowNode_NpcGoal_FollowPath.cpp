// 

#include "FlowGraph/Nodes/FlowNode_NpcGoal_FollowPath.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Subsystems/NpcPatrolRoutesSubsystem.h"

ENpcGoalStartResult UFlowNode_NpcGoal_FollowPath::Start()
{
	auto Result = Super::Start();
	auto NpcPatrolRouteSubsystem = UNpcPatrolRoutesSubsystem::Get(NpcPawn.Get());
	auto ActualParameters = GetParameters();
	
	if (!ensure(NpcPatrolRouteSubsystem) || !ensure(ActualParameters->RouteId.IsValid()))
		return ENpcGoalStartResult::Failed;

	auto PatrolRoute = NpcPatrolRouteSubsystem->StartPatrolRoute(NpcPawn.Get(), ActualParameters->RouteId,
		ActualParameters->bPreferClosestRoute, ActualParameters->bUsePathfinding);
	
	if (!PatrolRoute.IsValid())
		return ENpcGoalStartResult::Failed;

	BlackboardComponent->SetValueAsVector(BlackboardKeys->LocationToGoBBKey.SelectedKeyName, PatrolRoute.RoutePointLocation);

	if (ActualParameters->bStopAtPathPoint)
		UpdateStayAtPatrolPointTime();
	
	return Result;
}

ENpcGoalStartResult UFlowNode_NpcGoal_FollowPath::Restore(bool bInitialStart)
{
	ENpcGoalStartResult RestoreState = Super::Restore(bInitialStart);
	if (RestoreState == ENpcGoalStartResult::Failed)
		return RestoreState;
	
	auto NpcPatrolRouteSubsystem = UNpcPatrolRoutesSubsystem::Get(NpcPawn.Get());
	auto NpcRouteData = NpcPatrolRouteSubsystem->GetActivePatrolRoute(NpcPawn.Get());
	if (NpcRouteData.IsValid())
	{
		BlackboardComponent->SetValueAsVector(BlackboardKeys->LocationToGoBBKey.SelectedKeyName, NpcRouteData.RoutePointLocation);
		UpdateStayAtPatrolPointTime();
	}

	return NpcRouteData.IsValid() ? ENpcGoalStartResult::InProgress : ENpcGoalStartResult::Finished;
}

ENpcGoalAdvanceResult UFlowNode_NpcGoal_FollowPath::Advance(const FGameplayTagContainer& GoalExecutionResultTags)
{
	ENpcGoalAdvanceResult Result = Super::Advance(GoalExecutionResultTags);
	if (Result == ENpcGoalAdvanceResult::Failed)
		return Result;
	
	auto NpcPatrolRouteSubsystem = UNpcPatrolRoutesSubsystem::Get(NpcPawn.Get());
	FNpcPatrolRouteAdvanceResult NpcRouteAdvanceResult = NpcPatrolRouteSubsystem->GetNextPatrolRoutePoint(NpcPawn.Get());

	auto ActualParameters = GetParameters();
	
	if (NpcRouteAdvanceResult.LoopCount >= ActualParameters->Loops || ActualParameters->bCompleteOnReachingEdge && NpcRouteAdvanceResult.bReachedEdge)
		return ENpcGoalAdvanceResult::Completed;

	BlackboardComponent->SetValueAsVector(BlackboardKeys->LocationToGoBBKey.SelectedKeyName, NpcRouteAdvanceResult.NextLocation);
	if (ActualParameters->bStopAtPathPoint)
		UpdateStayAtPatrolPointTime();
	
	return ENpcGoalAdvanceResult::InProgress;
}

void UFlowNode_NpcGoal_FollowPath::Finish()
{
	auto NpcPatrolRouteSubsystem = UNpcPatrolRoutesSubsystem::Get(NpcPawn.Get());
	NpcPatrolRouteSubsystem->StopPatrolRoute(NpcPawn.Get());
	Super::Finish();
}

void UFlowNode_NpcGoal_FollowPath::UpdateStayAtPatrolPointTime() const
{
	if (auto NpcGameMode = Cast<INpcSystemGameMode>(NpcPawn->GetWorld()->GetAuthGameMode()))
	{
		auto ActualParameters = GetParameters();
		float GameTimeToRealTimeCoefficient = 3600.f / NpcGameMode->GetTimeRateSeconds();
		const float WaitTime = FMath::RandRange(ActualParameters->StayAtEachPatrolPointTimeMin * GameTimeToRealTimeCoefficient,
			ActualParameters->StayAtEachPatrolPointTimeMax * GameTimeToRealTimeCoefficient);
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->StayAtPatrolPointTimeBBKey.SelectedKeyName, WaitTime);
	}
}

const FNpcGoalParameters_FollowPath* UFlowNode_NpcGoal_FollowPath::GetParameters() const
{
	if (NpcGoalDataParameterId.IsValid())
	{
		const FNpcGoalParameters_FollowPath* Result = NpcComponent->GetNpcGoalParameters<FNpcGoalParameters_FollowPath>(NpcGoalDataParameterId);
		if (ensure(Result))
			return Result;
	}
	
	return &Parameters;
}
