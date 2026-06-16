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
	const auto& RememberedValueableItems = NpcPerceptionComponent->GetPerceivedValueableItems();
	
	for (const auto& ValueableItem : RememberedValueableItems)
	{
		bool bValidActor = ValueableItem.Actor.IsValid()
			&& (ActorTagsFilter.IsEmpty() || ActorTagsFilter.Matches(ValueableItem.ItemTags));
		
		if (!bValidActor)
			continue;

		ValueableItems.Add(ValueableItem.Actor.Get());
	}

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, ValueableItems);
}
