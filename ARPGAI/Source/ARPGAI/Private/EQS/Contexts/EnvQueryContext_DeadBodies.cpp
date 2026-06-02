#include "EQS/Contexts/EnvQueryContext_DeadBodies.h"

#include "Components/Controller/NpcPerceptionComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_DeadBodies::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                         FEnvQueryContextData& ContextData) const
{
	auto Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!Pawn)
		return;

	auto NpcPerceptionComponent = Pawn->GetController()->FindComponentByClass<UNpcPerceptionComponent>();
	if (!NpcPerceptionComponent)
		return;

	TArray<AActor*> DeadBodiesActors;
	const auto& CachedPerception = NpcPerceptionComponent->GetShortTermCharactersMemory();
	
	for (const auto& CharacterPerception : CachedPerception)
	{
		bool bValidActor = CharacterPerception.Key.IsValid() && !CharacterPerception.Value.IsAlive() 
			&& (CharacterPerception.Value.DetectionSource & EDetectionSource::VisualMemory) != EDetectionSource::None
			&& (AttitudesFilter.IsEmpty() || CharacterPerception.Value.Attitude.MatchesAny(AttitudesFilter))
			&& (ActorTagsFilter.IsEmpty() || ActorTagsFilter.Matches(CharacterPerception.Value.CharacterTags));
		if (!bValidActor)
			continue;

		DeadBodiesActors.Add(CharacterPerception.Key.Get());
	}

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, DeadBodiesActors);
}
