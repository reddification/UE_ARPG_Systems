


#include "EQS/Tests/EnvQueryTest_NpcReadyForInteraction.h"

#include "GameplayTagContainer.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcConversationComponent.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"

UEnvQueryTest_NpcReadyForInteraction::UEnvQueryTest_NpcReadyForInteraction(const FObjectInitializer& ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
	TestPurpose = EEnvTestPurpose::Type::Filter;
	FilterType = EEnvTestFilterType::Match;
}

void UEnvQueryTest_NpcReadyForInteraction::RunTest(FEnvQueryInstance& QueryInstance) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (QueryOwner == nullptr)
		return;
	
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		APawn* ItemPawn = Cast<APawn>(GetItemActor(QueryInstance, It.GetIndex()));
		if (ItemPawn == nullptr)
			continue;
		
		auto ConversationComponent = GetNpcConversationComponent(ItemPawn);
		if (ConversationComponent == nullptr)
			continue;
		
		FGameplayTag RefuseReason;
		It.SetScore(TestPurpose, FilterType, ConversationComponent->CanConversate(QueryOwner, RefuseReason), true);
	}
}

FText UEnvQueryTest_NpcReadyForInteraction::GetDescriptionDetails() const
{
	return Super::GetDescriptionDetails();
}
