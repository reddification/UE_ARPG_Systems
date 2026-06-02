#include "BehaviorEvaluators/BehaviorEvaluator_Combat.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcThreat.h"

UBehaviorEvaluatorConfig_Combat::UBehaviorEvaluatorConfig_Combat()
{
	OutCombatTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Combat, OutCombatTargetBBKey), AActor::StaticClass());
	OutAccumulatedDamageBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Combat, OutAccumulatedDamageBBKey));
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Combat::CreateEvaluator(UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Combat>(*BTComponent, this);
}

FBehaviorEvaluator_Combat::FBehaviorEvaluator_Combat(UBehaviorTreeComponent& OwnerComp,
	const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	CombatConfig = Cast<UBehaviorEvaluatorConfig_Combat>(Config);
	NpcThreatData.Reserve(8);
}

void FBehaviorEvaluator_Combat::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	float CombatDesireRaw = UpdatePerception();
	InterpolateUtility(CombatDesireRaw, DeltaTime);	
}

void FBehaviorEvaluator_Combat::OnActivated()
{
	Super::OnActivated();
	UpdatePerception();
}

float FBehaviorEvaluator_Combat::UpdatePerception()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_BehaviorEvaluator_Combat::UpdatePerception)

	auto PerceivedCharacters = PerceptionComponent->GetShortTermCharactersMemory();
	auto OwnerAliveCreature = Cast<INpcAliveCreature>(Pawn.Get());
	auto OwnerThreat = Cast<INpcThreat>(Pawn.Get());

	const FRichCurve* AccumulatedDamageCombatDesireDependency = CombatConfig->AccumulatedDamageToDesireScaleDependencyCurve.GetRichCurveConst();
	const FRichCurve* DistanceDependency = CombatConfig->HostileActorDistanceDesireDependencyCurve.GetRichCurveConst();
	const FRichCurve* DamageDependency = CombatConfig->DamageToDesireScaleDependencyCurve.GetRichCurveConst();
	const FRichCurve* StrengthAdvantageDependency = CombatConfig->StrengthAdvantageDesireDependencyCurve.GetRichCurveConst();
	const FRichCurve* ProtectionAdvantageDependency = CombatConfig->ProtectionAdvantageDesireDependencyCurve.GetRichCurveConst();
	
	const float AccumulatedDamageHealthFraction = PerceptionComponent->GetAccumulatedDamage(false) / OwnerAliveCreature->GetMaxHealth_NpcAliveCreature();
	const float CombatDesireHealthScale = AccumulatedDamageCombatDesireDependency->Eval(AccumulatedDamageHealthFraction);
	const float OwnerDamageOutput = OwnerThreat->GetDamageOutput_NpcThreat();
	const float OwnerProtection = OwnerThreat->GetAverageProtection_NpcThreat();
	const float OwnerMaxHealth = OwnerAliveCreature->GetMaxHealth_NpcAliveCreature();
	float CombatDesire = GetUtilityOffset();

	TArray<FCombatEvaluator_ActorPriority, TInlineAllocator<8>> PotentialEnemies;
	NpcThreatData.Reset();
	float AccumulatedDamage = 0.f;
	for (const auto& ActorPerception : PerceivedCharacters)
	{
		if (!ActorPerception.Value.IsAlive() || !ActorPerception.Value.IsHostile())
			continue;
		
		const float StrengthRatio = ActorPerception.Value.DamageOutput != 0.f ? OwnerDamageOutput / ActorPerception.Value.DamageOutput : OwnerDamageOutput;
		const float StrengthAdvantageScale = StrengthAdvantageDependency->Eval(StrengthRatio);
		float ProtectionRatio = ActorPerception.Value.Protection != 0.f ? OwnerProtection / ActorPerception.Value.Protection : OwnerProtection;
		float ProtectionAdvantageScale = ProtectionAdvantageDependency->Eval(ProtectionRatio);
		
		// 04.09.2025 (aki): deliberately using linear strength advantage dependency for distance and square dependency for damage
		const float ToughnessScale = StrengthAdvantageScale * ProtectionAdvantageScale;
		const float NormalizedIndividualDamage = ActorPerception.Value.ShortTermAccumulatedDamage / OwnerMaxHealth;
		float DistanceScore = DistanceDependency->Eval(ActorPerception.Value.Distance) * ToughnessScale;
		float DamageScore = DamageDependency->Eval(NormalizedIndividualDamage) * ToughnessScale * ToughnessScale;
		// FCombatEvaluator_ActorPriority PotentialEnemy { ActorPerception.Key.Get(), DistanceScore + DamageScore };
		
		auto IndividualCombatDesire = DistanceScore + DamageScore;
		if (ActorPerception.Value.DetectionSource == (ActorPerception.Value.DetectionSource & EDetectionSource::Ally)) 
			IndividualCombatDesire *= CombatConfig->AllyOnlyPerceptionScale;

		for (const auto& TagsBasedScale : CombatConfig->TagBasedScoreScales)
			if (TagsBasedScale.Filter.Matches(ActorPerception.Value.CharacterTags))
				IndividualCombatDesire *= TagsBasedScale.Value;
		
		NpcThreatData.Emplace(ActorPerception.Key, FNpcImmediateThreatData(IndividualCombatDesire, ActorPerception.Value.AttackRange));
		PotentialEnemies.Emplace(ActorPerception.Key.Get(), IndividualCombatDesire);
		CombatDesire += IndividualCombatDesire;
		
		if ((ActorPerception.Value.DetectionSource & EDetectionSource::Damage) != EDetectionSource::None)
			AccumulatedDamage += ActorPerception.Value.ShortTermAccumulatedDamage;
	}

	if (GetState() == EBehaviorEvaluatorState::Activated)
	{
		if (!PotentialEnemies.IsEmpty())
		{
			PotentialEnemies.Sort();
			ensure(PotentialEnemies[0].Actor != Pawn.Get());
			CombatLogicComponent->SetCurrentCombatTarget(PotentialEnemies[0].Actor, CombatConfig->BehaviorEvaluatorTag);
			Blackboard->SetValueAsObject(CombatConfig->OutCombatTargetBBKey.SelectedKeyName, PotentialEnemies[0].Actor);
		}
		else
		{
			CombatLogicComponent->ClearCurrentCombatTarget(CombatConfig->BehaviorEvaluatorTag);
			Blackboard->SetValueAsObject(CombatConfig->OutCombatTargetBBKey.SelectedKeyName, nullptr);
		}
		
		Blackboard->SetValueAsFloat(CombatConfig->OutAccumulatedDamageBBKey.SelectedKeyName, AccumulatedDamage / OwnerMaxHealth);
		CombatLogicComponent->UpdateImmediateThreats(MoveTemp(NpcThreatData));
	}

	return CombatDesire * CombatDesireHealthScale;
}

void FBehaviorEvaluator_Combat::Cleanup()
{
	Super::Cleanup();
	if (Blackboard.IsValid() && CombatConfig.IsValid())
	{
		Blackboard->ClearValue(CombatConfig->OutCombatTargetBBKey.SelectedKeyName);
		Blackboard->ClearValue(CombatConfig->OutAccumulatedDamageBBKey.SelectedKeyName);
	}
	
	if (CombatLogicComponent.IsValid())
	{
		CombatLogicComponent->ClearCurrentCombatTarget(CombatConfig->BehaviorEvaluatorTag);
		CombatLogicComponent->ClearEnemiesData();
	}
}
