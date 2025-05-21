#include "EQS/Contexts/EQSContext_Threats.h"

#include "Components/NpcCombatLogicComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEQSContext_Threats::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const AActor* MobQuerier = Cast<AActor>(QueryInstance.Owner.Get());
	if (!IsValid(MobQuerier))
	{
		return;
	}

	if (UNpcCombatLogicComponent* MobComponent = MobQuerier->FindComponentByClass<UNpcCombatLogicComponent>())
	{
		const FNpcActiveThreatsContainer& ActiveThreats = MobComponent->GetActiveThreats();
		TArray<const AActor*> Actors;
		Actors.Reserve(ActiveThreats.Num());
		for (const auto& ActiveThreat : ActiveThreats)
		{
			if (const AActor* ThreatActor = ActiveThreat.Key.Get())
			{
				Actors.Emplace(ThreatActor);
			}
		}
		
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Actors);
	}
}
