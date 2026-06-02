#include "BehaviorEvaluators/BehaviorEvaluator_Retreat.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/NpcThreat.h"
#include "Subsystems/NpcSquadSubsystem.h"

UBehaviorEvaluatorConfig_Retreat::UBehaviorEvaluatorConfig_Retreat()
{
	OutPrimaryRetreatTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Retreat, OutPrimaryRetreatTargetBBKey), AActor::StaticClass());
	OutAccumulatedDamageBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Retreat, OutAccumulatedDamageBBKey));
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Retreat::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Retreat>(*BTComponent, this);
}

FBehaviorEvaluator_Retreat::FBehaviorEvaluator_Retreat(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config) 
	: FBehaviorEvaluator_Base(OwnerComp, Config)
{
	RetreatConfig = Cast<UBehaviorEvaluatorConfig_Retreat>(Config);
	ensure(RetreatConfig.IsValid());
	NpcThreatData.Reserve(10);
}

void FBehaviorEvaluator_Retreat::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	float RawUtility = UpdatePerception();
	InterpolateUtility(RawUtility, DeltaTime);
}

float FBehaviorEvaluator_Retreat::UpdatePerception()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FBehaviorEvaluator_Retreat::UpdatePerception)
	
	NpcThreatData.Reset();	
	auto Allies = UNpcSquadSubsystem::Get(AIController.Get())->GetAllies(Pawn.Get(), true);
	auto NpcAliveCreature = Cast<INpcAliveCreature>(Pawn);
	auto NpcSelfThreat = Cast<INpcThreat>(Pawn);
	
	FVector NpcLocation = AIController->GetPawn()->GetActorLocation();
	const float MyToughness = NpcSelfThreat->GetDamageOutput_NpcThreat() + NpcSelfThreat->GetAverageProtection_NpcThreat();

	float CalmnessFactor = 0.f;
	auto CalmnessDependency = RetreatConfig->AllyDistanceCalmnessDependency.GetRichCurveConst();
	for (const auto& Ally : Allies)
	{
		float Distance = (Ally->GetActorLocation() - NpcLocation).Size();
		CalmnessFactor += CalmnessDependency->Eval(Distance);
	}

	float RetreatDesire = GetUtilityOffset();
	auto ThreatScoreDistanceDependency = RetreatConfig->ThreatDistanceScoreDependency.GetRichCurveConst(); 
	auto DamageScoreDependency = RetreatConfig->AccumulatedDamageScoreDependency.GetRichCurveConst();
	auto HealthToFearScaleDependencyCurve = RetreatConfig->HealthFractionToFearScaleDependency.GetRichCurveConst();
	const float MaxHealth = NpcAliveCreature->GetMaxHealth_NpcAliveCreature();
	const float HealthNormalized = NpcAliveCreature->GetHealth_NpcAliveCreature() / MaxHealth;
	const float HealthFearScale = HealthToFearScaleDependencyCurve->Eval(HealthNormalized);
	const auto& PerceptionData = PerceptionComponent->GetShortTermCharactersMemory();
	TArray<FFearData, TInlineAllocator<10>> FearData;
	float AccumulatedDamage = 0.f;
	for (const auto& ActorPerception : PerceptionData)
	{
		if (!ActorPerception.Value.IsHostile() || !ActorPerception.Value.IsAlive())
			continue;
			
		float LocalFearScore = 0.f;
		if ((ActorPerception.Value.DetectionSource & EDetectionSource::VisualMemory) != EDetectionSource::None)
		{
			float EnemyToughness = ActorPerception.Value.DamageOutput + ActorPerception.Value.Protection;
			float StrengthAdvantage = MyToughness / (EnemyToughness != 0.f ? EnemyToughness : 1.f);
			if (StrengthAdvantage <= RetreatConfig->StrengthDisadvantageActivation)
			{
				float DistanceBasedScore = ThreatScoreDistanceDependency->Eval(ActorPerception.Value.Distance) / StrengthAdvantage;
				LocalFearScore += DistanceBasedScore;
			}
		}
		
		if ((ActorPerception.Value.DetectionSource & EDetectionSource::Damage) != EDetectionSource::None)
		{
			LocalFearScore += DamageScoreDependency->Eval(ActorPerception.Value.ShortTermAccumulatedDamage / MaxHealth);
			AccumulatedDamage += ActorPerception.Value.ShortTermAccumulatedDamage;
		}
		
		for (const auto& TagsBasedScale : RetreatConfig->TagBasedScoreScales)
			if (TagsBasedScale.Filter.Matches(ActorPerception.Value.CharacterTags))
				LocalFearScore *= TagsBasedScale.Value;
		
		RetreatDesire += LocalFearScore;
		if (GetState() == EBehaviorEvaluatorState::Activated)
		{
			FearData.Add({ ActorPerception.Key.Get(), LocalFearScore });
			NpcThreatData.Add(ActorPerception.Key.Get(), FNpcImmediateThreatData( LocalFearScore, ActorPerception.Value.AttackRange));
		}
	}

	if (GetState() == EBehaviorEvaluatorState::Activated)
	{
		FearData.Sort();
		if (!FearData.IsEmpty())
		{
			CombatLogicComponent->SetCurrentCombatTarget(FearData[0].Actor, RetreatConfig->BehaviorEvaluatorTag);
			Blackboard->SetValueAsObject(RetreatConfig->OutPrimaryRetreatTargetBBKey.SelectedKeyName, FearData[0].Actor);
			Blackboard->SetValueAsFloat(RetreatConfig->OutAccumulatedDamageBBKey.SelectedKeyName, AccumulatedDamage / MaxHealth);
		}
		else
		{
			CombatLogicComponent->ClearCurrentCombatTarget(RetreatConfig->BehaviorEvaluatorTag);
			Blackboard->ClearValue(RetreatConfig->OutPrimaryRetreatTargetBBKey.SelectedKeyName);
			Blackboard->ClearValue(RetreatConfig->OutAccumulatedDamageBBKey.SelectedKeyName);
		}
		
		CombatLogicComponent->UpdateImmediateThreats(NpcThreatData);
	}

	return RetreatDesire * HealthFearScale - CalmnessFactor;
}

void FBehaviorEvaluator_Retreat::OnActivated()
{
	Super::OnActivated();
	UpdatePerception();
}

void FBehaviorEvaluator_Retreat::Cleanup()
{
	Super::Cleanup();
	if (RetreatConfig.IsValid())
	{
		if (Blackboard.IsValid())
		{
			Blackboard->SetValueAsObject(RetreatConfig->OutPrimaryRetreatTargetBBKey.SelectedKeyName, nullptr);
			Blackboard->SetValueAsFloat(RetreatConfig->OutAccumulatedDamageBBKey.SelectedKeyName, 0.f);
		}
	
		if (CombatLogicComponent.IsValid())
			CombatLogicComponent->ClearCurrentCombatTarget(RetreatConfig->BehaviorEvaluatorTag);
	}
}