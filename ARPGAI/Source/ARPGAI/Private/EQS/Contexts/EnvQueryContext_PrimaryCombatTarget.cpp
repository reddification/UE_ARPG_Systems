// 


#include "EQS/Contexts/EnvQueryContext_PrimaryCombatTarget.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcCombatLogicComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_PrimaryCombatTarget::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                          FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
		return;

	if (auto NpcCombatLogicComponent = GetNpcCombatLogicComponent(QuerierPawn))
		if (auto CurrentTarget = NpcCombatLogicComponent->GetPrimaryTargetActor())
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, CurrentTarget);
}
