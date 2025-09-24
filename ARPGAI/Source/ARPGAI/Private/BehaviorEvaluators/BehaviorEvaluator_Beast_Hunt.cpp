// 


#include "BehaviorEvaluators/BehaviorEvaluator_Beast_Hunt.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBehaviorEvaluator_Beast_Hunt::UBehaviorEvaluator_Beast_Hunt()
{
	OutPreyTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluator_Beast_Hunt, OutPreyTargetBBKey), AActor::StaticClass());
	TickInterval = 0.25f;
}

void UBehaviorEvaluator_Beast_Hunt::Evaluate()
{
	Super::Evaluate();
	auto PerceptionComponent = AIController->GetAIPerceptionComponent();
	auto SightSenseId = UAISense::GetSenseID(UAISense_Sight::StaticClass());

	AActor* ClosestAliveCreature = nullptr;
	float ClosestAliveCreatureDistance = FLT_MAX;
	FVector NpcLocation = AIController->GetPawn()->GetActorLocation();
	const ANavigationData* NavData = nullptr;
	if (bUsePathFinding)
	{
		auto NavSys = UNavigationSystemV1::GetCurrent(AIController.Get());
		NavData = NavSys->GetNavDataForProps(AIController->GetNavAgentPropertiesRef(), NpcLocation);
	}
	
	for (auto DataIt = PerceptionComponent->GetPerceptualDataConstIterator(); DataIt; ++DataIt)
	{
		if (DataIt->Value.Target.IsValid())
			UE_VLOG(AIController.Get(), LogARPGAI_BE_Hunt, VeryVerbose, TEXT("Processing perception for %s"), *DataIt->Value.Target->GetName());
		
		// scan hearing and damage, collect visual targets
		for (const auto& AIStimulus : DataIt.Value().LastSensedStimuli)
		{
			if (AIStimulus.IsExpired())
			{
				UE_VLOG(AIController.Get(), LogARPGAI_BE_Hunt, VeryVerbose, TEXT("Stimulus of type %s is expired for %s"),
					 *AIStimulus.Type.Name.ToString(), *DataIt->Value.Target->GetName());
				
				continue;
			}

			if (AIStimulus.Type == SightSenseId)
			{
				auto PerceivedActor = DataIt->Value.Target.Get();
				auto AliveCreatureNpc = Cast<INpcAliveCreature>(PerceivedActor);
				if (!AliveCreatureNpc->IsNpcActorAlive() || !PreysIds.HasTag(AliveCreatureNpc->GetNpcAliveCreatureId()))
					continue;

				float DistanceToActor = FLT_MAX;
				
				if (bUsePathFinding)
				{
					auto NavSys = UNavigationSystemV1::GetCurrent(AIController.Get());
					if (NavData)
					{
						FPathFindingQuery Query(AIController.Get(), *NavData, NpcLocation, PerceivedActor->GetActorLocation());
						FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, EPathFindingMode::Hierarchical);
						DistanceToActor = PathFindingResult.Path->GetLength();

						if (PathFindingResult.Result == ENavigationQueryResult::Type::Fail)
						{
							UE_VLOG(AIController->GetOwner(), LogARPGAI_BE_Hunt, Warning, TEXT("Can't find path to target"));
						}
					}
				}
				else
				{
					DistanceToActor = (NpcLocation - PerceivedActor->GetActorLocation()).Size();
				}

				if (DistanceToActor < ClosestAliveCreatureDistance)
				{
					ClosestAliveCreatureDistance = DistanceToActor;
					ClosestAliveCreature = PerceivedActor;
				}
			}
		}
	}

	if (ClosestAliveCreature != nullptr)
	{
		
	}
	else
	{
		ChangeUtility(-UtilityDecayRate * TickInterval);
	}
}
