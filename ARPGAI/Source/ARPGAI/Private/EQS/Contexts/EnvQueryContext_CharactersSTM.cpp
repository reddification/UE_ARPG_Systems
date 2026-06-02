#include "EQS/Contexts/EnvQueryContext_CharactersSTM.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_CharactersSTM::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                    FEnvQueryContextData& ContextData) const
{
	auto Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!Pawn)
		return;

	auto ShortTermMemoryComponent = GetNpcShortTermMemoryComponent(Pawn);
	if (!ShortTermMemoryComponent)
		return;

	TArray<AActor*> Actors;
	const auto& CachedPerception = ShortTermMemoryComponent->GetShortTermCharactersMemory();

	const bool bHasAliveStateFilter = MustBeAliveStateFilter.IsSet();
	const bool bAliveStateFilter = MustBeAliveStateFilter.GetValue(); 
	
	for (const auto& CharacterSTM : CachedPerception)
	{
		if (bHasAliveStateFilter && CharacterSTM.Value.bAlive != bAliveStateFilter)
			continue;
		
		if (!CharacterIdFilter.IsEmpty() && !CharacterSTM.Value.CharacterId.MatchesAny(CharacterIdFilter))
			continue;
		
		if (!AttitudeFilter.IsEmpty() && !CharacterSTM.Value.Attitude.MatchesAny(CharacterIdFilter))
			continue;
		
		bool bDetectionSourcePass = true;
		for (const auto& DetectionSourceFilter : DetectionSourcesFilter)
		{
			if (CharacterSTM.Value.HasDetectionSource(DetectionSourceFilter.Key) != DetectionSourceFilter.Value)
			{
				bDetectionSourcePass = false;
				break;
			}
		}
		
		if (!bDetectionSourcePass)
			continue;
		
		if (!CharacterStateFilter.IsEmpty() && !CharacterStateFilter.Matches(CharacterSTM.Value.CharacterTags))
			continue;
		
		Actors.Add(CharacterSTM.Key.Get());
	}

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Actors);
}
