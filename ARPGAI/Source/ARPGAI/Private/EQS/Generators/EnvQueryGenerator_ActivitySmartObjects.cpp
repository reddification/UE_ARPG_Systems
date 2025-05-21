// 


#include "EQS/Generators/EnvQueryGenerator_ActivitySmartObjects.h"

#include "EnvQueryItemType_SmartObject.h"
#include "GameplayTagAssetInterface.h"
#include "SmartObjectComponent.h"
#include "Components/Controller/NpcActivityComponent.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

UEnvQueryGenerator_ActivitySmartObjects::UEnvQueryGenerator_ActivitySmartObjects()
{
	ItemType = UEnvQueryItemType_SmartObject::StaticClass();
	QueryOriginContext = UEnvQueryContext_Querier::StaticClass();
}

void UEnvQueryGenerator_ActivitySmartObjects::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	APawn* QueryOwner = Cast<APawn>(QueryInstance.Owner.Get());
	if (QueryOwner == nullptr || QueryOwner->GetController() == nullptr)
		return;

	auto NpcActivityComponent = QueryOwner->GetController()->FindComponentByClass<UNpcActivityComponent>();
	if (!NpcActivityComponent)
		return;

	auto ActiveSmartObjectGoal = Cast<UNpcGoalUseSmartObject>(NpcActivityComponent->GetActiveGoal());
	if (!ensure(ActiveSmartObjectGoal))
		return;
	
	UWorld* World = GEngine->GetWorldFromContextObject(QueryOwner, EGetWorldErrorMode::LogAndReturnNull);
	USmartObjectSubsystem* SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(World);

	if (SmartObjectSubsystem == nullptr)
		return;

	TArray<FSmartObjectRequestResult> FoundSlots;
	TArray<FVector> OriginLocations;
	QueryInstance.PrepareContext(QueryOriginContext, OriginLocations);
	int32 NumberOfSuccessfulQueries = 0;
	
	FSmartObjectRequestFilter GoalSmartObjectRequestFilter;
	auto Parameters = ActiveSmartObjectGoal->GetParameters(QueryOwner);
	GoalSmartObjectRequestFilter.ActivityRequirements = Parameters.IntentionFilter;
	FVector QueryBoxExtent(Parameters.LocationSearchRadius);
	for (const FVector& Origin : OriginLocations)
	{
		FBox QueryBox(Origin - QueryBoxExtent, Origin + QueryBoxExtent);
		FSmartObjectRequest Request(QueryBox, GoalSmartObjectRequestFilter);

		// @todo note that with this approach, if there's more than one Origin being used for generation we can end up 
		// with duplicates in AllResults
		NumberOfSuccessfulQueries += SmartObjectSubsystem->FindSmartObjects(Request, FoundSlots) ? 1 : 0;
	}

	if (NumberOfSuccessfulQueries > 1)
	{
		FoundSlots.Sort([](const FSmartObjectRequestResult& A, const FSmartObjectRequestResult& B)
		{
			return A.SlotHandle < B.SlotHandle;
		});
	}

	TArray<FSmartObjectSlotEQSItem> AllResults;
	AllResults.Reserve(FoundSlots.Num());

	FSmartObjectSlotHandle PreviousSlotHandle;
	bool bCheckSmartObjectActor = !Parameters.SmartObjectActorFilter.IsEmpty();
	const auto& SmartObjects = SmartObjectSubsystem->GetSmartObjectContainer();
	for (const FSmartObjectRequestResult& SlotResult : FoundSlots)
	{
		if (NumberOfSuccessfulQueries > 1 && SlotResult.SlotHandle == PreviousSlotHandle)
		{
			// skip duplicates
			continue;
		}
		
		PreviousSlotHandle = SlotResult.SlotHandle;

		if (bCheckSmartObjectActor)
		{
			auto SmartObjectTagActor = Cast<IGameplayTagAssetInterface>(SmartObjects.GetSmartObjectComponent(SlotResult.SmartObjectHandle)->GetOwner());
			if (!SmartObjectTagActor)
				continue;

			FGameplayTagContainer SmartObjectActorTags;
			SmartObjectTagActor->GetOwnedGameplayTags(SmartObjectActorTags);
			if (!Parameters.SmartObjectActorFilter.Matches(SmartObjectActorTags))
				continue;
		}
		
		if (bOnlyClaimable == false || SmartObjectSubsystem->CanBeClaimed(SlotResult.SlotHandle))
		{
			const FTransform& SlotTransform = SmartObjectSubsystem->GetSlotTransformChecked(SlotResult.SlotHandle);
			AllResults.Emplace(SlotTransform.GetLocation(), SlotResult.SmartObjectHandle, SlotResult.SlotHandle);
		}
	}

	QueryInstance.AddItemData<UEnvQueryItemType_SmartObject>(AllResults);
}
