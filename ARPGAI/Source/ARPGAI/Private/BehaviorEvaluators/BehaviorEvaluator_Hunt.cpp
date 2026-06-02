#include "BehaviorEvaluators/BehaviorEvaluator_Hunt.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcCombatInterface.h"
#include "Interfaces/NpcThreat.h"

UBehaviorEvaluatorConfig_Hunt::UBehaviorEvaluatorConfig_Hunt()
{
	OutPreyTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Hunt, OutPreyTargetBBKey), AActor::StaticClass());
	OuHuntStateTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Hunt, OuHuntStateTagsBBKey)));
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Hunt::CreateEvaluator(UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Hunt>(*BTComponent, this);
}

FBehaviorEvaluator_Hunt::FBehaviorEvaluator_Hunt(UBehaviorTreeComponent& OwnerComp,
                                                 const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	HuntConfig = Cast<UBehaviorEvaluatorConfig_Hunt>(Config);
	auto OwnerThreat = Cast<INpcThreat>(Pawn.Get());
	if (ensure(OwnerThreat))
		MyStrength = OwnerThreat->GetDamageOutput_NpcThreat() + OwnerThreat->GetAverageProtection_NpcThreat();
}

void FBehaviorEvaluator_Hunt::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	TRACE_CPUPROFILER_EVENT_SCOPE(FBehaviorEvaluator_Hunt::Update)
	
	FVector NpcLocation = AIController->GetPawn()->GetActorLocation();
	const ANavigationData* NavData = nullptr;
	UNavigationSystemV1* NavSys = nullptr;
	if (HuntConfig->bUsePathFinding)
	{
		NavSys = UNavigationSystemV1::GetCurrent(AIController.Get());
		NavData = NavSys->GetNavDataForProps(AIController->GetNavAgentPropertiesRef(), NpcLocation);
	}

	float HuntDesire = GetUtilityOffset();
	const auto& ShortTermMemory = PerceptionComponent->GetShortTermCharactersMemory();
	TArray<FHuntingPrey> Preys;
	TArray<FHuntingThreat> Threats;

	for (const auto& ActorPerception : ShortTermMemory)
	{
		if (!ActorPerception.Value.IsAlive() || ActorPerception.Value.bAlly)
			continue;
		
		if (!HuntConfig->PreyTagsFilter.IsEmpty() && !HuntConfig->PreyTagsFilter.Matches(ActorPerception.Value.CharacterTags))
			continue;
		
		if ((ActorPerception.Value.DetectionSource & EDetectionSource::VisualMemory) != EDetectionSource::None)
		{
			if (ActorPerception.Value.CharacterId.MatchesAny(HuntConfig->PreysIds))
			{
				float DistanceToActor = ActorPerception.Value.Distance;
				
				if (HuntConfig->bUsePathFinding)
				{
					if (NavData)
					{
						FPathFindingQuery Query(AIController.Get(), *NavData, NpcLocation, ActorPerception.Key->GetActorLocation());
						FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, EPathFindingMode::Hierarchical);

						if (PathFindingResult.Result == ENavigationQueryResult::Type::Success)
							DistanceToActor = PathFindingResult.Path->GetLength();
						else 
							{ UE_VLOG(AIController.Get(), LogARPGAI_BE_Hunt, Warning, TEXT("Can't find path to target")); }

					}
				}

				FHuntingPrey Prey(ActorPerception.Key.Get(), DistanceToActor, 1.f);
				Preys.Add(Prey);
			}
			else if (ActorPerception.Value.IsHostile())
			{
				FHuntingThreat Threat(ActorPerception.Key.Get(), MyStrength / ActorPerception.Value.DamageOutput);
				Threats.Add(Threat);
			}
		}

		if ((ActorPerception.Value.DetectionSource & EDetectionSource::Audio) != EDetectionSource::None)
		{
			// idk
		}
	}

	AActor* NewPrey = nullptr;
	if (!Preys.IsEmpty())
	{
		auto DistanceDependency = HuntConfig->PreyDistanceToScoreDependency.GetRichCurveConst();
		auto ThreatDistanceDependency = HuntConfig->PreyThreatsProximityDependency.GetRichCurveConst();
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
		
		if (Preys.Num() > 1)
			Preys.Sort([](const FHuntingPrey& Prey1, const FHuntingPrey& Prey2){ return Prey1.Score > Prey2.Score; });
		
		NewPrey = Preys[0].Actor;
	}
	
	if (GetState() == EBehaviorEvaluatorState::Activated)
	{
		if (NewPrey != CurrentPrey)
		{
			CurrentPrey = NewPrey;
			Blackboard->SetValueAsObject(HuntConfig->OutPreyTargetBBKey.SelectedKeyName, CurrentPrey.Get());
			if (CurrentPrey.IsValid())
			{
				if (CurrentPrey != CombatLogicComponent->GetPrimaryTargetActor())
					CombatLogicComponent->SetCurrentCombatTarget(CurrentPrey.Get(), HuntConfig->BehaviorEvaluatorTag);
			}
			else
			{
				CombatLogicComponent->ClearCurrentCombatTarget(HuntConfig->BehaviorEvaluatorTag);
			}
		}
	}
	else
	{
		CurrentPrey = NewPrey;
	}
		
	InterpolateUtility(HuntDesire, DeltaTime);
}

void FBehaviorEvaluator_Hunt::OnActivated()
{
	Super::OnActivated();
	if (Blackboard.IsValid() && HuntConfig.IsValid())
		Blackboard->SetValueAsObject(HuntConfig->OutPreyTargetBBKey.SelectedKeyName, CurrentPrey.Get());
	
	if (auto NpcCombatant = Cast<INpcCombatInterface>(Pawn.Get()))
		if (ensure(!ActorKilledDelegateHandle.IsValid()))
			ActorKilledDelegateHandle = NpcCombatant->KilledActorEvent_NpcCombatant.AddRaw(this, &FBehaviorEvaluator_Hunt::OnActorKilled);
}

void FBehaviorEvaluator_Hunt::Cleanup()
{
	Super::Cleanup();
	if (Blackboard.IsValid() && HuntConfig.IsValid())
		Blackboard->ClearValue(HuntConfig->OutPreyTargetBBKey.SelectedKeyName);
	
	if (CombatLogicComponent.IsValid())
		CombatLogicComponent->ClearCurrentCombatTarget(HuntConfig->BehaviorEvaluatorTag);
	
	if (auto NpcCombatant = Cast<INpcCombatInterface>(Pawn.Get()))
	{
		if (ensure(ActorKilledDelegateHandle.IsValid()))
		{
			NpcCombatant->KilledActorEvent_NpcCombatant.Remove(ActorKilledDelegateHandle);
			ActorKilledDelegateHandle.Reset();
		}
	}
	
	CurrentPrey.Reset();
}

void FBehaviorEvaluator_Hunt::OnActorKilled(AActor* Actor, const FGameplayTag& LastHitType)
{
	if (IsValid(Actor) && Actor == CurrentPrey)
	{
		if (Blackboard.IsValid() && HuntConfig.IsValid())
		{
			if (HuntConfig->PreyKilledEventTag.IsValid())
			{
				FGameplayTagContainer CurrentGoalTags = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(HuntConfig->OuHuntStateTagsBBKey.SelectedKeyName);
				CurrentGoalTags.AddTag(HuntConfig->PreyKilledEventTag);
				Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(HuntConfig->OuHuntStateTagsBBKey.SelectedKeyName, CurrentGoalTags);
			}
		}
	}
}
