#include "EQS/Contexts/EnvQueryContext_ValuableItems.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_ValuableItems::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                    FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	auto Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!Pawn)
		return;

	auto NpcPerceptionComponent = Pawn->GetController()->FindComponentByClass<UNpcPerceptionComponent>();
	if (!NpcPerceptionComponent)
		return;

	TArray<AActor*> ValueableItems;
	const auto& CachedPerception = NpcPerceptionComponent->GetPerceivedValueableItems();
	
	for (const auto& ActorPerception : CachedPerception)
	{
		bool bValidActor = ActorPerception.Key.IsValid()
			&& (ActorTagsFilter.IsEmpty() || ActorTagsFilter.Matches(ActorPerception.Value.ItemTags));
		
		if (!bValidActor)
			continue;

		ValueableItems.Add(ActorPerception.Key.Get());
	}

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, ValueableItems);
}
