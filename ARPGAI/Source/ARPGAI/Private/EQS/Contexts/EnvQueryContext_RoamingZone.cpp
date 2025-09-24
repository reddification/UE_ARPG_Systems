// 


#include "EQS/Contexts/EnvQueryContext_RoamingZone.h"

#include "Components/NpcAreasComponent.h"
#include "Components/NpcComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_RoamingZone::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                  FEnvQueryContextData& ContextData) const
{
	auto Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!Pawn)
		return;

	TArray<AActor*> NpcAreaActors;
	
	if (auto NpcComponent = Pawn->FindComponentByClass<UNpcAreasComponent>())
	{
		const auto& NpcAreas = NpcComponent->GetNpcAreas();
		for (const auto& NpcAreaType : NpcAreas)
		{
			for (const auto& NpcArea : NpcAreaType.Value.NpcAreas)
				NpcAreaActors.Add(const_cast<AActor*>(Cast<const AActor>(NpcArea.GetObject())));
		}
	}
	
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, NpcAreaActors);
}
