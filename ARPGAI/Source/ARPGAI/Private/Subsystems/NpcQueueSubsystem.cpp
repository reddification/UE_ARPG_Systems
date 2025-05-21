// 


#include "Subsystems/NpcQueueSubsystem.h"

#include "Components/NpcQueueComponent.h"

void UNpcQueueSubsystem::RegisterQueue(UNpcQueueComponent* NpcQueueComponent, const FGameplayTag& QueueId)
{
	NpcQueues.Add(QueueId, NpcQueueComponent);
}

void UNpcQueueSubsystem::UnregisterQueue(const FGameplayTag& QueueId)
{
	NpcQueues.Remove(QueueId);
}

UNpcQueueComponent* UNpcQueueSubsystem::GetQueue(const FGameplayTag& QueueId)
{
	auto NpcQueuePtr = NpcQueues.Find(QueueId);
	return NpcQueuePtr ? NpcQueuePtr->Get() : nullptr;
}
