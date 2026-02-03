// 


#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_Hunt.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/Npc.h"
#include "Interfaces/Threat.h"
#include "Perception/AIPerceptionComponent.h"

UBTService_BehaviorEvaluator_Hunt::UBTService_BehaviorEvaluator_Hunt()
{
	NodeName = "Behavior evaluator: hunt";
	OutPreyTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_Hunt, OutPreyTargetBBKey), AActor::StaticClass());
}

void UBTService_BehaviorEvaluator_Hunt::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto BTMemory = reinterpret_cast<FBTMemory_BE_Hunt*>(NodeMemory);
	auto OwnerThreat = Cast<IThreat>(OwnerComp.GetAIOwner()->GetPawn());
	BTMemory->MyStrength = OwnerThreat->GetStrength() + OwnerThreat->GetAverageProtection();
}

// what beast should account for when picking a prey:
// 1. is prey alive
// 2. distance to prey - prefer closer
// 3. STRONGER hostile actors in proximity (other predators, humans)
// 4. personal hunger (i.e. if the beast is very hungry, even if a human is nearby - fuck it - go for the lunch)
void UBTService_BehaviorEvaluator_Hunt::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_BehaviorEvaluator_Hunt::TickNode)
	
	auto AIController = OwnerComp.GetAIOwner();
	auto PerceptionComponent = Cast<UNpcPerceptionComponent>( AIController->GetAIPerceptionComponent());
	
	FVector NpcLocation = AIController->GetPawn()->GetActorLocation();
	const ANavigationData* NavData = nullptr;
	UNavigationSystemV1* NavSys = nullptr;
	if (bUsePathFinding)
	{
		NavSys = UNavigationSystemV1::GetCurrent(AIController);
		NavData = NavSys->GetNavDataForProps(AIController->GetNavAgentPropertiesRef(), NpcLocation);
	}

	auto BTMemory = reinterpret_cast<FBTMemory_BE_Hunt*>(NodeMemory);
	float HuntDesire = BTMemory->GetUtilityRegressionOffset();
	auto PerceptionCache = PerceptionComponent->GetAnimatePerceptionData();
	TArray<FHuntingPrey> Preys;
	TArray<FHuntingThreat> Threats;

	for (const auto& ActorPerception : PerceptionCache)
	{
		if (!ActorPerception.Value.IsAlive() || ActorPerception.Value.bAlly)
			continue;
		
		if ((ActorPerception.Value.DetectionSource & NpcCombatEvaluation::Visual) != 0)
		{
			if (PreysIds.HasTagExact(ActorPerception.Value.NpcId))
			{
				float DistanceToActor = ActorPerception.Value.Distance;
				
				if (bUsePathFinding)
				{
					if (NavData)
					{
						FPathFindingQuery Query(AIController, *NavData, NpcLocation, ActorPerception.Key->GetActorLocation());
						FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, EPathFindingMode::Hierarchical);
						DistanceToActor = PathFindingResult.Path->GetLength();

						if (PathFindingResult.Result == ENavigationQueryResult::Type::Fail)
						{
							UE_VLOG(AIController->GetOwner(), LogARPGAI_BE_Hunt, Warning, TEXT("Can't find path to target"));
						}
					}
				}

				FHuntingPrey Prey(ActorPerception.Key.Get(), DistanceToActor, 1.f);
				Preys.Add(Prey);
			}
			else if (ActorPerception.Value.IsHostile())
			{
				FHuntingThreat Threat(ActorPerception.Key.Get(), BTMemory->MyStrength / ActorPerception.Value.Strength);
				Threats.Add(Threat);
			}
		}

		if (ActorPerception.Value.DetectionSource & NpcCombatEvaluation::Audio)
		{
			// idk
		}
	}

	if (!Preys.IsEmpty())
	{
		auto DistanceDependency = PreyDistanceToScoreDependency.GetRichCurveConst();
		auto ThreatDistanceDependency = PreyThreatsProximityDependency.GetRichCurveConst();
		for (auto& Prey : Preys)
		{
			Prey.Score = DistanceDependency->Eval(Prey.Distance);
			for (const auto& Threat : Threats)
			{
				float DistanceToThreat = (Prey.Actor->GetActorLocation() - Threat.Actor->GetActorLocation()).Size();
				Prey.Score -= ThreatDistanceDependency->Eval(DistanceToThreat) / Threat.StrengthAdvantage;
			}

			HuntDesire += Prey.Score;
		}
	}
	
	Preys.Sort([](const FHuntingPrey& Prey1, const FHuntingPrey& Prey2){ return Prey1.Score > Prey2.Score; });
	
	BTMemory->Prey = Preys[0].Actor;
	if (BTMemory->bActive)
	{
		if (BTMemory->Prey.IsValid())
		{
			if (BTMemory->Prey != BTMemory->CombatLogicComponent->GetPrimaryTargetActor())
				BTMemory->CombatLogicComponent->SetCurrentCombatTarget(BTMemory->Prey.Get(), BehaviorEvaluatorTag);
		}
		else 
			BTMemory->CombatLogicComponent->ClearCurrentCombatTarget();
	}
		
	ChangeUtility(HuntDesire, OwnerComp.GetBlackboardComponent(), Interval, BTMemory);
}

void UBTService_BehaviorEvaluator_Hunt::InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::InitiateBehaviorState(BTComponent);
	auto BTMemory = reinterpret_cast<FBTMemory_BE_Hunt*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	ensure(BTMemory->Prey.IsValid());
	BTComponent->GetBlackboardComponent()->SetValueAsObject(OutPreyTargetBBKey.SelectedKeyName, BTMemory->Prey.Get());	
}

void UBTService_BehaviorEvaluator_Hunt::FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	if (IsValid(BTComponent))
	{
		auto BTMemory = reinterpret_cast<FBTMemory_BE_Hunt*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
		BTComponent->GetBlackboardComponent()->ClearValue(OutPreyTargetBBKey.SelectedKeyName);
		BTMemory->Prey = nullptr;
	}
	
	Super::FinalizeBehaviorState(BTComponent);
}

FString UBTService_BehaviorEvaluator_Hunt::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out]Prey BB: %s\n%s"), *OutPreyTargetBBKey.SelectedKeyName.ToString(), *Super::GetStaticDescription());
}
