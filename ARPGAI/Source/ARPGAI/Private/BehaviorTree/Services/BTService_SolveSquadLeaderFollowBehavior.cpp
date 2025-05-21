// 


#include "BehaviorTree/Services/BTService_SolveSquadLeaderFollowBehavior.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SolveSquadLeaderFollowBehavior::UBTService_SolveSquadLeaderFollowBehavior()
{
	NodeName = "Solve squad leader follow behavior";
	OutCloseToSquadLeaderBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SolveSquadLeaderFollowBehavior, OutCloseToSquadLeaderBBKey));
	SquadLeaderBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SolveSquadLeaderFollowBehavior, SquadLeaderBBKey), AActor::StaticClass());
	bCallTickOnSearchStart = true;
}

void UBTService_SolveSquadLeaderFollowBehavior::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto SquadLeader = Cast<AActor>(Blackboard->GetValueAsObject(SquadLeaderBBKey.SelectedKeyName));
	if (!ensure(SquadLeader))
		return;

	if (SquadLeader->GetVelocity().SizeSquared() > 10.f)
	{
		Blackboard->SetValueAsBool(OutCloseToSquadLeaderBBKey.SelectedKeyName, false);
		return;
	}

	auto NpcPawn = OwnerComp.GetAIOwner()->GetPawn();
	const float DistanceThreshold = SquadLeaderThresholdDistanceToJoinStatic * SquadLeaderThresholdDistanceToJoinStatic;
	bool bCloseToSquadLeader = (NpcPawn->GetActorLocation() - SquadLeader->GetActorLocation()).SizeSquared() < DistanceThreshold;
	Blackboard->SetValueAsBool(OutCloseToSquadLeaderBBKey.SelectedKeyName, bCloseToSquadLeader);
}

FString UBTService_SolveSquadLeaderFollowBehavior::GetStaticDescription() const
{
	return FString::Printf(TEXT("Set %s to true\nWhen squad leader %s is not moving\nAnd querier distance to leader is < %.2f\n%s"),
		*OutCloseToSquadLeaderBBKey.SelectedKeyName.ToString(), *SquadLeaderBBKey.SelectedKeyName.ToString(), SquadLeaderThresholdDistanceToJoinStatic,
		*Super::GetStaticDescription());
}
