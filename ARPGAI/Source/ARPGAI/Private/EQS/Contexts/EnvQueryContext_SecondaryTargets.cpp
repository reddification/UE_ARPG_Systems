#include "EQS/Contexts/EnvQueryContext_SecondaryTargets.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcCombatLogicComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_SecondaryTargets::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                       FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
		return;

	if (UNpcCombatLogicComponent* NpcCombatLogicComponent = GetNpcCombatLogicComponent(QuerierPawn))
	{
		auto PrimaryTarget = NpcCombatLogicComponent->GetPrimaryTargetActor();
		const FNpcActiveThreatsContainer& ActiveTargets = NpcCombatLogicComponent->GetActiveThreats();
		TArray<const AActor*> Actors;
		Actors.Reserve(ActiveTargets.Num());
		for (const auto& ActiveThreat : ActiveTargets)
			if (ActiveThreat.Key.IsValid() && ActiveThreat.Key != PrimaryTarget)
				Actors.Emplace(ActiveThreat.Key.Get());
		
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Actors);
	}
}
