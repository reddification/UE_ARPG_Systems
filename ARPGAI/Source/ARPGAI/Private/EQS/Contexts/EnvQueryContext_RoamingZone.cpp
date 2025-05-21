// 


#include "EQS/Contexts/EnvQueryContext_RoamingZone.h"

#include "Components/NpcComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_RoamingZone::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                  FEnvQueryContextData& ContextData) const
{
	auto Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!Pawn)
		return;

	if (auto NpcComponent = Pawn->FindComponentByClass<UNpcComponent>())
		if (auto DesignatedZoneComponent = NpcComponent->GetDesignatedZone())
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, DesignatedZoneComponent->GetOwner());
}
