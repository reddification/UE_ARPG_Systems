#include "EQS/Generators/EnvQueryGenerator_ActorsFromContext.h"

#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

#define LOCTEXT_NAMESPACE "EnvQueryGenerator"

UEnvQueryGenerator_ActorsFromContext::UEnvQueryGenerator_ActorsFromContext(const FObjectInitializer& ObjectInitializer)
 	: Super(ObjectInitializer)
{
	QueryContext = UEnvQueryContext::StaticClass();
	ItemType = UEnvQueryItemType_Actor::StaticClass();
}

void UEnvQueryGenerator_ActorsFromContext::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	TArray<AActor*> Actors;
	QueryInstance.PrepareContext(QueryContext, Actors);
	QueryInstance.AddItemData<UEnvQueryItemType_Actor>(Actors);
}

FText UEnvQueryGenerator_ActorsFromContext::GetDescriptionTitle() const
{
	return FText::Format(LOCTEXT("EQS_ActorFromContext", "Actor context is {0}"), UEnvQueryTypes::DescribeContext(QueryContext));
}

#undef LOCTEXT_NAMESPACE