#include "EQS/Tests/EnvQueryTest_RememberedTraits.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/Controller/NpcMemoryComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"

UEnvQueryTest_RememberedTraits::UEnvQueryTest_RememberedTraits()
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
	TestPurpose = EEnvTestPurpose::Type::Filter;
	FilterType = EEnvTestFilterType::Match;
	SetWorkOnFloatValues(false);
}

void UEnvQueryTest_RememberedTraits::RunTest(FEnvQueryInstance& QueryInstance) const
{
	APawn* QueryOwner = Cast<APawn>(QueryInstance.Owner.Get());
	if (QueryOwner == nullptr)
		return;
	
	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);
	bool MatchValue = BoolValue.GetValue();
	
	auto OwnerAttitudesComponent = GetNpcLongTermMemoryComponent(QueryOwner);
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());
		FGameplayTagContainer RememberedTraits = OwnerAttitudesComponent->GetRememberedActorTraits(ItemActor);
		bool bMatches = RememberedTraitsFilter.Matches(RememberedTraits);
		It.SetScore(TestPurpose, FilterType, bMatches, MatchValue);
	}
}

FText UEnvQueryTest_RememberedTraits::GetDescriptionTitle() const
{
	return FText::FromString(TEXT("Check remembered traits for actor"));
}

FText UEnvQueryTest_RememberedTraits::GetDescriptionDetails() const
{
	return FText::FromString(RememberedTraitsFilter.GetDescription());
}
