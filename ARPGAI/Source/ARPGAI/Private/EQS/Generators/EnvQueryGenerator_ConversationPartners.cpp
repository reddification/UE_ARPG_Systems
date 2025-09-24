// 


#include "EQS/Generators/EnvQueryGenerator_ConversationPartners.h"

#include "Components/NpcComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Subsystems/NpcRegistrationSubsystem.h"

UEnvQueryGenerator_ConversationPartners::UEnvQueryGenerator_ConversationPartners()
{
	ItemType = UEnvQueryItemType_Actor::StaticClass();
	SearchRangeValue.DefaultValue = 2500;
}

void UEnvQueryGenerator_ConversationPartners::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	APawn* QueryOwner = Cast<APawn>(QueryInstance.Owner.Get());
	if (QueryOwner == nullptr || QueryOwner->GetController() == nullptr)
		return;

	SearchRangeValue.BindData(QueryOwner, QueryInstance.QueryID);
	const float SearchRange = SearchRangeValue.GetValue();

	TArray<FGameplayTagQuery> NpcTagsFilters;
	NpcTagsFilters.Add(GameplayTagQuery);
	
	auto NpcActivityComponent = QueryOwner->GetController()->FindComponentByClass<UNpcFlowComponent>();
	const FNpcGoalParameters_Conversate* ConversateParameters = NpcActivityComponent->GetParameters<FNpcGoalParameters_Conversate>();
	if (ConversateParameters)
		if (!ConversateParameters->ConversationPartnerTagsFilter.IsEmpty())
			NpcTagsFilters.Add(ConversateParameters->ConversationPartnerTagsFilter);
	
	auto NpcSubsystem = QueryOwner->GetWorld()->GetSubsystem<UNpcRegistrationSubsystem>();
	auto NpcsInRange = NpcSubsystem->GetNpcsInRange(QueryOwner->GetActorLocation(), SearchRange, NpcTagsFilters);
	
	TArray<AActor*> AllResults;
	auto OwnerNpcComponent = QueryOwner->FindComponentByClass<UNpcComponent>();
	for (const auto* NpcInRange : NpcsInRange)
		if (OwnerNpcComponent != NpcInRange)
			AllResults.Add(NpcInRange->GetOwner());

	QueryInstance.AddItemData<UEnvQueryItemType_Actor>(AllResults);
}
