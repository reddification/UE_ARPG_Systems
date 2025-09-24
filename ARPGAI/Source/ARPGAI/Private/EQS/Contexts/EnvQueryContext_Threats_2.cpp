// 


#include "EQS/Contexts/EnvQueryContext_Threats_2.h"

#include "Components/Controller/NpcPerceptionComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_Threats_2::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                FEnvQueryContextData& ContextData) const
{
	auto Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!Pawn)
		return;

	auto NpcPerceptionComponent = Pawn->GetController()->FindComponentByClass<UNpcPerceptionComponent>();
	if (!NpcPerceptionComponent)
		return;

	TArray<AActor*> Threats;
	const auto& CachedPerception = NpcPerceptionComponent->GetAnimatePerceptionData();
	for (const auto& CharacterPerception : CachedPerception)
	{
		bool bValidThreat = CharacterPerception.Value.IsAlive() && CharacterPerception.Value.IsHostile()
			&& (CharacterPerception.Value.DetectionSource & NpcCombatEvaluation::Visual) != 0;
		if (!bValidThreat)
			continue;

		Threats.Add(CharacterPerception.Key.Get());
	}

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Threats);
}
