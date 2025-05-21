


#include "EQS/Generators/EnvQueryGenerator_GuardZoneAllies.h"

#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EQS/Contexts/EQSContext_AllyNpcs.h"

#define LOCTEXT_NAMESPACE "EnvQueryGenerator"

UEnvQueryGenerator_GuardZoneAllies::UEnvQueryGenerator_GuardZoneAllies(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	QueryContext = UEQSContext_AllyNpcs::StaticClass();
	ItemType = UEnvQueryItemType_Actor::StaticClass();
}

void UEnvQueryGenerator_GuardZoneAllies::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	TArray<AActor*> GuardZoneAllies;
	QueryInstance.PrepareContext(QueryContext, GuardZoneAllies);
	QueryInstance.AddItemData<UEnvQueryItemType_Actor>(GuardZoneAllies);
}

FText UEnvQueryGenerator_GuardZoneAllies::GetDescriptionTitle() const
{
	return FText::Format(LOCTEXT("CurrentActor", "Current Actor is {0}"), UEnvQueryTypes::DescribeContext(QueryContext));
}

#undef LOCTEXT_NAMESPACE