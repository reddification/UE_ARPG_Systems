#include "Subsystems/WorldStateSubsystem.h"

UWorldStateSubsystem* UWorldStateSubsystem::Get(const UObject* WorldContextObject)
{
	if (!ensure(WorldContextObject != nullptr))
		return nullptr;
	
	auto World = WorldContextObject->GetWorld();
	if (ensure(World))
	{
		auto GameInstance = World->GetGameInstance();
		if (ensure(GameInstance))
			return GameInstance->GetSubsystem<UWorldStateSubsystem>();
	}

	return nullptr;
}

void UWorldStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// TODO load world state
}

void UWorldStateSubsystem::ChangeWorldState(const FGameplayTagContainer& TagsContainer, bool bAdd, bool bBroadcastChanges)
{
	if (!IsValid(GetWorld()) || GetWorld()->bIsTearingDown)
		return;
	
	if (bAdd)
		WorldState.AppendTags(TagsContainer);
	else
		WorldState.RemoveTags(TagsContainer);
	
	if (bBroadcastChanges && WorldStateChangedEvent.IsBound())
		WorldStateChangedEvent.Broadcast(WorldState);
}

bool UWorldStateSubsystem::IsAtWorldState(const FGameplayTagQuery& TestWorldState) const
{
	return TestWorldState.IsEmpty() || TestWorldState.Matches(WorldState);
}

void UWorldStateSubsystem::Load()
{
	WorldState.Reset();
}
