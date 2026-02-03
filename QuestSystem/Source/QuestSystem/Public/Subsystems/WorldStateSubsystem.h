#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WorldStateSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FWorldStateChangedEvent, const FGameplayTagContainer& NewGameState);

UCLASS()
class QUESTSYSTEM_API UWorldStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UWorldStateSubsystem* Get(const UObject* WorldContextObject);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION(BlueprintCallable)
	void ChangeWorldState(const FGameplayTagContainer& TagsContainer, bool bAdd = true, bool bBroadcastChanges = true);
	
	UFUNCTION(BlueprintCallable)
	bool IsAtWorldState(const FGameplayTagQuery& TestWorldState) const;
	
	UFUNCTION(BlueprintCallable)
	const FGameplayTagContainer& GetWorldState() const { return WorldState; }

	void Load();

	mutable FWorldStateChangedEvent WorldStateChangedEvent;
	
private:
	FGameplayTagContainer WorldState;
};
