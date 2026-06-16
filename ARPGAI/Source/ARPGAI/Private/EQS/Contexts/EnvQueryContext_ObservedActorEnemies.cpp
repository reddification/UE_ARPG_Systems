#include "EQS/Contexts/EnvQueryContext_ObservedActorEnemies.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Interfaces/NpcThreat.h"

void UEnvQueryContext_ObservedActorEnemies::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	auto Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!Pawn)
		return;

	auto NpcComponent = GetNpcComponent(Pawn);
	if (!NpcComponent)
		return;
	
	auto Target = NpcComponent->GetStoredActor(StoredActorTag);
	if (Target == nullptr)
		return;
	
	auto CombatInfoProvider = Cast<INpcThreat>(Target);
	if (!CombatInfoProvider)
		return;

	TArray<AActor*> CurrentEnemies = CombatInfoProvider->GetCurrentEnemies_NpcThreat();
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, CurrentEnemies);
}
