// 


#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_Combat.h"

#include "AIController.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Threat.h"

using namespace NpcCombatEvaluation;

UBTService_BehaviorEvaluator_Combat::UBTService_BehaviorEvaluator_Combat()
{
	NodeName = "Behavior evaluator: combat";
	OutCombatTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_Combat, OutCombatTargetBBKey), AActor::StaticClass());
	OutAccumulatedDamageBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_Combat, OutAccumulatedDamageBBKey));
}

void UBTService_BehaviorEvaluator_Combat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                   float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	float CombatDesireRaw = UpdatePerception(OwnerComp, BTMemory);
	ChangeUtility(CombatDesireRaw, Blackboard, DeltaSeconds, BTMemory);
}

float UBTService_BehaviorEvaluator_Combat::UpdatePerception(UBehaviorTreeComponent& OwnerComp, const FBTMemory_BehaviorEvaluator_Base* BTMemory) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_BehaviorEvaluator_Combat::UpdatePerception)

#if WITH_EDITOR
	if (!BTMemory->PerceptionComponent.IsValid())
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Error, TEXT("Shit's fucked up no BTMemory in combat perception update"));
		return 0.f;
	}
#endif
	
	auto PerceivedCharacters = BTMemory->PerceptionComponent->GetAnimatePerceptionData();
	auto NpcPawn = OwnerComp.GetAIOwner()->GetPawn();
	auto OwnerAliveCreature = Cast<INpcAliveCreature>(NpcPawn);
	auto OwnerThreat = Cast<IThreat>(NpcPawn);
	auto Blackboard = OwnerComp.GetBlackboardComponent();

	const FRichCurve* AccumulatedDamageCombatDesireDependency = nullptr;
	const FRichCurve* DistanceDependency = nullptr; 
	const FRichCurve* DamageDependency = nullptr; 	
	const FRichCurve* StrengthAdvantageDependency = nullptr;

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_BehaviorEvaluator_Combat::UpdatePerception::GetDependencies)
		
		AccumulatedDamageCombatDesireDependency = AccumulatedDamageToCombatDesireScaleDependencyCurve.GetRichCurveConst();
		DistanceDependency = HostileActorDistanceCombatDesireDependencyCurve.GetRichCurveConst();
		DamageDependency = DamageToCombatDesireScaleDependencyCurve.GetRichCurveConst();
		StrengthAdvantageDependency = StrengthAdvantageCombatDesireDependencyCurve.GetRichCurveConst();
	}
	
	const float AccumulatedDamageHealthFraction = BTMemory->PerceptionComponent->GetAccumulatedDamage() / OwnerAliveCreature->GetMaxHealth();
	const float CombatDesireHealthScale = AccumulatedDamageCombatDesireDependency->Eval(AccumulatedDamageHealthFraction);
	const float OwnerStrength = OwnerThreat->GetStrength() + OwnerThreat->GetAverageProtection();
	float CombatDesire = BTMemory->GetUtilityRegressionOffset();

	TArray<FCombatEvaluator_ActorPriority> PotentialEnemies;
	PotentialEnemies.Reserve(5);
	float AccumulatedDamage = 0.f;
	for (const auto& ActorPerception : PerceivedCharacters)
	{
		if (!ActorPerception.Value.IsAlive() || !ActorPerception.Value.IsHostile())
			continue;
		
		const float StrengthAdvantageScale = StrengthAdvantageDependency->Eval(OwnerStrength / ActorPerception.Value.Strength);
		// 04.09.2025 (aki): deliberately using linear strength advantage dependency for distance and square dependency for damage
		float DistanceScore = DistanceDependency->Eval(ActorPerception.Value.Distance) * StrengthAdvantageScale;
		float DamageScore = DamageDependency->Eval(ActorPerception.Value.AccumulatedDamage) * StrengthAdvantageScale * StrengthAdvantageScale;
		// FCombatEvaluator_ActorPriority PotentialEnemy { ActorPerception.Key.Get(), DistanceScore + DamageScore };
		PotentialEnemies.Emplace(ActorPerception.Key.Get(), DistanceScore + DamageScore);
		CombatDesire += DistanceScore + DamageScore;
		if ((ActorPerception.Value.DetectionSource) == (ActorPerception.Value.DetectionSource & EDetectionSource::Ally)) 
			CombatDesire *= AllyOnlyPerceptionScale;

		if (ActorPerception.Value.DetectionSource & EDetectionSource::Damage)
			AccumulatedDamage += ActorPerception.Value.AccumulatedDamage;
	}

	if (BTMemory->bActive)
	{
		if (!PotentialEnemies.IsEmpty())
		{
			PotentialEnemies.Sort();
			Blackboard->SetValueAsObject(OutCombatTargetBBKey.SelectedKeyName, PotentialEnemies[0].Actor);
		}
		else
		{
			Blackboard->SetValueAsObject(OutCombatTargetBBKey.SelectedKeyName, nullptr);
		}
		
		Blackboard->SetValueAsFloat(OutAccumulatedDamageBBKey.SelectedKeyName, AccumulatedDamage);
	}

	return CombatDesire * CombatDesireHealthScale;
}

void UBTService_BehaviorEvaluator_Combat::InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::InitiateBehaviorState(BTComponent);
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	UpdatePerception(*BTComponent, BTMemory);
}

void UBTService_BehaviorEvaluator_Combat::FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::FinalizeBehaviorState(BTComponent);
	if (BTComponent)
	{
		if (auto Blackboard = BTComponent->GetBlackboardComponent())
		{
			Blackboard->SetValueAsObject(OutCombatTargetBBKey.SelectedKeyName, nullptr);
			Blackboard->SetValueAsFloat(OutAccumulatedDamageBBKey.SelectedKeyName, 0.f);
		}
	}
}

FString UBTService_BehaviorEvaluator_Combat::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}
