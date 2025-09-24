#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_Retreat.h"
#include "AIController.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Threat.h"
#include "Perception/AIPerceptionComponent.h"
#include "Subsystems/NpcSquadSubsystem.h"

UBTService_BehaviorEvaluator_Retreat::UBTService_BehaviorEvaluator_Retreat()
{
	NodeName = "Behavior evaluator: retreat";
	OutPrimaryRetreatTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_Retreat, OutPrimaryRetreatTargetBBKey), AActor::StaticClass());
	OutAccumulatedDamageBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_Retreat, OutAccumulatedDamageBBKey));
}

void UBTService_BehaviorEvaluator_Retreat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                    float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	float RawUtility = UpdatePerception(OwnerComp, BTMemory);
	ChangeUtility(RawUtility, Blackboard, Interval, BTMemory);
}

void UBTService_BehaviorEvaluator_Retreat::InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::InitiateBehaviorState(BTComponent);
	UpdatePerception(*BTComponent, reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this))));
}

void UBTService_BehaviorEvaluator_Retreat::FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	if (IsValid(BTComponent))
	{
		if (auto Blackboard = BTComponent->GetBlackboardComponent())
		{
			Blackboard->SetValueAsObject(OutPrimaryRetreatTargetBBKey.SelectedKeyName, nullptr);
			Blackboard->SetValueAsFloat(OutAccumulatedDamageBBKey.SelectedKeyName, 0.f);
		}
	}
	
	Super::FinalizeBehaviorState(BTComponent);
}

float UBTService_BehaviorEvaluator_Retreat::UpdatePerception(UBehaviorTreeComponent& OwnerComp, const FBTMemory_BehaviorEvaluator_Base* BTMemory) const
{
#if WITH_EDITOR
	if (!BTMemory->PerceptionComponent.IsValid())
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Error, TEXT("Shit's fucked up no BTMemory in retreat perception update"));
		return 0.f;
	}
#endif
	
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_BehaviorEvaluator_Retreat::UpdatePerception)
	auto AIController = OwnerComp.GetAIOwner();
	auto PerceptionComponent = Cast<UNpcPerceptionComponent>(AIController->GetAIPerceptionComponent());
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto Pawn = AIController->GetPawn();
	auto Allies = UNpcSquadSubsystem::Get(AIController)->GetAllies(Pawn, false, true);
	auto NpcAliveCreature = Cast<INpcAliveCreature>(Pawn);
	auto NpcSelfThreat = Cast<IThreat>(Pawn);
	
	FVector NpcLocation = AIController->GetPawn()->GetActorLocation();
	const float MyStrength = NpcSelfThreat->GetStrength() + NpcSelfThreat->GetAverageProtection();

	float CalmnessFactor = 0.f;
	auto CalmnessDependency = AllyDistanceCalmnessDependency.GetRichCurveConst();
	for (const auto& Ally : Allies)
	{
		float Distance = (Ally->GetActorLocation() - NpcLocation).Size();
		CalmnessFactor += CalmnessDependency->Eval(Distance);
	}

	float RetreatDesire = BTMemory->GetUtilityRegressionOffset();
	auto ThreatScoreDistanceDependency = ThreatDistanceScoreDependency.GetRichCurveConst(); 
	auto DamageScoreDependency = AccumulatedDamageScoreDependency.GetRichCurveConst();
	auto HealthToFearScaleDependencyCurve = HealthFractionToFearScaleDependency.GetRichCurveConst();
	const float HealthNormalized = NpcAliveCreature->GetHealth() / NpcAliveCreature->GetMaxHealth();
	const float HealthFearScale = HealthToFearScaleDependencyCurve->Eval(HealthNormalized);
	auto PerceptionData = PerceptionComponent->GetAnimatePerceptionData();
	TArray<FFearData> FearData;
	float AccumulatedDamage = 0.f;
	FearData.Reserve(10);
	for (const auto& ActorPerception : PerceptionData)
	{
		if (!ActorPerception.Value.IsHostile() || !ActorPerception.Value.IsAlive())
			continue;
			
		float LocalFearScore = 0.f;
		if (ActorPerception.Value.DetectionSource & EDetectionSource::Visual)
		{
			float StrengthAdvantage = MyStrength / ActorPerception.Value.Strength;
			if (StrengthAdvantage <= StrengthDisadvantageActivation)
			{
				float DistanceBasedScore = ThreatScoreDistanceDependency->Eval(ActorPerception.Value.Distance) / StrengthAdvantage;
				LocalFearScore += DistanceBasedScore;
			}
		}
		
		if (((ActorPerception.Value.DetectionSource & EDetectionSource::Damage) != 0) && ActorPerception.Value.IsAlive())
		{
			LocalFearScore += DamageScoreDependency->Eval(ActorPerception.Value.AccumulatedDamage);
			AccumulatedDamage += ActorPerception.Value.AccumulatedDamage;
		}
		
		RetreatDesire += LocalFearScore;
		if (BTMemory->bActive)
			FearData.Add({ ActorPerception.Key.Get(), LocalFearScore });
	}

	if (BTMemory->bActive && !FearData.IsEmpty())
	{
		FearData.Sort();
		Blackboard->SetValueAsObject(OutPrimaryRetreatTargetBBKey.SelectedKeyName, FearData[0].Actor);
		Blackboard->SetValueAsFloat(OutAccumulatedDamageBBKey.SelectedKeyName, AccumulatedDamage);
	}

	return RetreatDesire * HealthFearScale - CalmnessFactor;
}

FString UBTService_BehaviorEvaluator_Retreat::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}
