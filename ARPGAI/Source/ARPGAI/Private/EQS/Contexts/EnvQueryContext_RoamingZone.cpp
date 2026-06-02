// 


#include "EQS/Contexts/EnvQueryContext_RoamingZone.h"

#include "Activities/NpcComponentsHelpers.h"
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
	
	if (auto NpcComponent = GetNpcAreasComponent(Pawn))
	{
		const auto& NpcAreas = NpcComponent->GetNpcAreas();
		if (NpcAreas.Num() > 0)
		{
			NpcAreaActors.Reserve(NpcAreas.Num());
		
			for (const auto& NpcAreaType : NpcAreas)
				for (const auto& NpcArea : NpcAreaType.Value.NpcAreas)
					NpcAreaActors.Add(const_cast<AActor*>(Cast<const AActor>(NpcArea.GetObject())));
		}
	}
	
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, NpcAreaActors);
}
