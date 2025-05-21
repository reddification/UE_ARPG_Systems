// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "NpcQueueSubsystem.generated.h"

class UNpcQueueComponent;
/**
 * 
 */
UCLASS()
class ARPGAI_API UNpcQueueSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterQueue(UNpcQueueComponent* NpcQueueComponent, const FGameplayTag& QueueId);
	void UnregisterQueue(const FGameplayTag& QueueId);

	UFUNCTION(BlueprintCallable)
	UNpcQueueComponent* GetQueue(const FGameplayTag& QueueId);

private:
	TMap<FGameplayTag, TWeakObjectPtr<UNpcQueueComponent>> NpcQueues;
};
