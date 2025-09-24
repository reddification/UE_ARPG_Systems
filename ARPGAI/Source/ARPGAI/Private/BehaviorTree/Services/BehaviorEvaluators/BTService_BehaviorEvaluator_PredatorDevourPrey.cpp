// 
#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_PredatorDevourPrey.h"

#include "Components/Controller/NpcPerceptionComponent.h"

UBTService_BehaviorEvaluator_PredatorDevourPrey::UBTService_BehaviorEvaluator_PredatorDevourPrey()
{
	NodeName = "Behavior evaluator: predator devour prey";
	DevourTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_PredatorDevourPrey, DevourTargetBBKey), AActor::StaticClass());
}

void UBTService_BehaviorEvaluator_PredatorDevourPrey::TickNode(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	float DevourDesireRaw = UpdatePerception(OwnerComp, BTMemory);
	ChangeUtility(DevourDesireRaw, Blackboard, DeltaSeconds, BTMemory);
}

float UBTService_BehaviorEvaluator_PredatorDevourPrey::UpdatePerception(UBehaviorTreeComponent& OwnerComp,
	const FBTMemory_BehaviorEvaluator_Base* BTMemory) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_BehaviorEvaluator_PredatorDevourPrey::UpdatePerception)
	float Desire = BTMemory->GetUtilityRegressionOffset();
	auto PerceptionData = BTMemory->PerceptionComponent->GetAnimatePerceptionData();
	if (PerceptionData.IsEmpty())
		return Desire;

	auto DistanceToDeadPreyScoreDependency = DistanceToDeadPreyDependencyCurve.GetRichCurveConst();
	auto DistanceToAliveEnemyScoreDependency = DistanceToAliveEnemyDependencyCurve.GetRichCurveConst();

	float ClosestTargetDistance = FLT_MAX;
	AActor* ClosestTarget = nullptr;
	
	for (const auto& PerceptionDataEntry : PerceptionData)
	{
		if (!PerceptionDataEntry.Value.IsHostile() || PerceptionDataEntry.Value.IsAlive())
			continue;
		
		Desire += PerceptionDataEntry.Value.IsAlive()
			? DistanceToAliveEnemyScoreDependency->Eval(PerceptionDataEntry.Value.Distance)
			: DistanceToDeadPreyScoreDependency->Eval(PerceptionDataEntry.Value.Distance);

		if (BTMemory->bActive && PerceptionDataEntry.Value.Distance < ClosestTargetDistance)
		{
			ClosestTargetDistance = PerceptionDataEntry.Value.Distance;
			ClosestTarget = PerceptionDataEntry.Key.Get();
		}
	}

	if (BTMemory->bActive)
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(DevourTargetBBKey.SelectedKeyName, ClosestTarget);

	return Desire;
}

void UBTService_BehaviorEvaluator_PredatorDevourPrey::InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::InitiateBehaviorState(BTComponent);
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	UpdatePerception(*BTComponent, BTMemory);
}

void UBTService_BehaviorEvaluator_PredatorDevourPrey::FinalizeBehaviorState(
	UBehaviorTreeComponent* BTComponent) const
{
	if (auto Blackboard = BTComponent->GetBlackboardComponent())
		Blackboard->ClearValue(DevourTargetBBKey.SelectedKeyName);	
	
	Super::FinalizeBehaviorState(BTComponent);
}
