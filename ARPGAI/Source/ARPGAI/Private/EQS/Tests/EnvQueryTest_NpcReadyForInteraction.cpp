


#include "EQS/Tests/EnvQueryTest_NpcReadyForInteraction.h"

#include "GameplayTagContainer.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "Interfaces/Npc.h"

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
		AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());
		auto NpcItem = Cast<INpc>(ItemActor);
		if (!NpcItem)
			continue;
		
		FGameplayTag RefuseReason;
		if(NpcItem->CanConversate(QueryOwner, RefuseReason))
		{
			It.SetScore(TestPurpose, FilterType, true, true);
			//It.ForceItemState(EEnvItemStatus::Passed);	
		}
		else
		{
			It.SetScore(TestPurpose, FilterType, false, true);
			//It.ForceItemState(EEnvItemStatus::Failed);
		}
	}
}

FText UEnvQueryTest_NpcReadyForInteraction::GetDescriptionDetails() const
{
	return Super::GetDescriptionDetails();
}
