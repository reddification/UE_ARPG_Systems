#include "BehaviorEvaluators/v2/BehaviorEvaluator_Combat_Heal.h"
#include "Components/Controller/NpcPerceptionComponent.h"

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Combat_Heal::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Combat_Heal>(*BTComponent, this);
}

FBehaviorEvaluator_Combat_Heal::FBehaviorEvaluator_Combat_Heal(UBehaviorTreeComponent& BTComponent, const UBehaviorEvaluatorConfig_Base* Config) 
	: Super(BTComponent, Config)
{
	HealConfig = Cast<UBehaviorEvaluatorConfig_Combat_Heal>(Config);
}

float FBehaviorEvaluator_Combat_Heal::Evaluate()
{
	Super::Evaluate();
	TRACE_CPUPROFILER_EVENT_SCOPE(FBehaviorEvaluator_CombatBase::Evaluate)
	
	float HealDesire = CalculateStatePressure();
	if (HealDesire <= 0.f)
		return HealDesire;
	
	FRelativeOperationContext RelativeOperationData = GetRelativeOperationContext();
	FActorScoresContainer Enemies; 
	const auto& CharactersPerception = PerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& CharacterPerception : CharactersPerception)
	{
		ExecuteEntityOperations(CharacterPerception.Key.Get(), CharacterPerception.Value, RelativeOperationData,
			Enemies, HealConfig->EnemiesEvaluationParameters);
	}
	
	float EnemyBasedReluctance = GetEntitiesAggregatedScore(Enemies, OperationsConfig->EnemiesEvaluationParameters);
	return HealDesire - EnemyBasedReluctance;
}