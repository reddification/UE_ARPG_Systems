// 


#include "BehaviorEvaluators/BehaviorEvaluator_PredatorDevourPrey.h"

#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"

UBehaviorEvaluatorConfig_PredatorDevourPrey::UBehaviorEvaluatorConfig_PredatorDevourPrey()
{
	DevourTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_PredatorDevourPrey, DevourTargetBBKey), AActor::StaticClass());
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_PredatorDevourPrey::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_PredatorDevourPrey>(*BTComponent, this);
}

FBehaviorEvaluator_PredatorDevourPrey::FBehaviorEvaluator_PredatorDevourPrey(UBehaviorTreeComponent& OwnerComp,
                                                                             const UBehaviorEvaluatorConfig_Base* BaseConfig) : Super(OwnerComp, BaseConfig)
{
	Config = Cast<UBehaviorEvaluatorConfig_PredatorDevourPrey>(BaseConfig);
}

void FBehaviorEvaluator_PredatorDevourPrey::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	float DevourDesireRaw = UpdatePerception();
	InterpolateUtility(DevourDesireRaw, DeltaTime);
}

float FBehaviorEvaluator_PredatorDevourPrey::UpdatePerception() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_BehaviorEvaluator_PredatorDevourPrey::UpdatePerception)
	float Desire = GetUtilityOffset();
	auto PerceptionData = PerceptionComponent->GetShortTermCharactersMemory();
	if (PerceptionData.IsEmpty())
		return Desire;

	auto DistanceToDeadPreyScoreDependency = Config->DistanceToDeadPreyDependencyCurve.GetRichCurveConst();
	auto DistanceToAliveEnemyScoreDependency = Config->DistanceToAliveEnemyDependencyCurve.GetRichCurveConst();

	float ClosestTargetDistance = FLT_MAX;
	AActor* ClosestTarget = nullptr;
	
	for (const auto& PerceptionDataEntry : PerceptionData)
	{
		if (!Config->PreyFilter.IsEmpty() && !Config->PreyFilter.Matches(PerceptionDataEntry.Value.CharacterTags))
			continue;
		
		Desire += PerceptionDataEntry.Value.IsAlive()
			? DistanceToAliveEnemyScoreDependency->Eval(PerceptionDataEntry.Value.Distance)
			: DistanceToDeadPreyScoreDependency->Eval(PerceptionDataEntry.Value.Distance);

		if (GetState() == EBehaviorEvaluatorState::Activated && PerceptionDataEntry.Value.Distance < ClosestTargetDistance)
		{
			ClosestTargetDistance = PerceptionDataEntry.Value.Distance;
			ClosestTarget = PerceptionDataEntry.Key.Get();
		}
	}

	if (GetState() == EBehaviorEvaluatorState::Activated)
	{
		CombatLogicComponent->SetCurrentCombatTarget(ClosestTarget, Config->BehaviorEvaluatorTag);
		Blackboard->SetValueAsObject(Config->DevourTargetBBKey.SelectedKeyName, ClosestTarget);
	}
	
	return Desire;
}

void FBehaviorEvaluator_PredatorDevourPrey::OnActivated()
{
	Super::OnActivated();
	UpdatePerception();
}

void FBehaviorEvaluator_PredatorDevourPrey::Cleanup()
{
	Super::Cleanup();
	if (Blackboard.IsValid() && Config.IsValid())
		Blackboard->ClearValue(Config->DevourTargetBBKey.SelectedKeyName);
	
	if (CombatLogicComponent.IsValid() && BaseConfig.IsValid())
		CombatLogicComponent->ClearCurrentCombatTarget(BaseConfig->BehaviorEvaluatorTag);
	
	
}