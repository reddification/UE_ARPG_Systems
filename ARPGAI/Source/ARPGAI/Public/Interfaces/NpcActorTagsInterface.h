#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "NpcActorTagsInterface.generated.h"

UINTERFACE()
class UNpcActorTagsInterface : public UInterface
{
	GENERATED_BODY()
};

class ARPGAI_API INpcActorTagsInterface
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_TwoParams(FTagsChangedEvent_NPC, AActor* OwnerActor, const FGameplayTagContainer& NewTags);
	
public:
	virtual FGameplayTagContainer GetTags_NPC() const = 0;

	virtual void GiveTag_NPC(const FGameplayTag& NewTag) = 0;
	virtual void GiveTags_NPC(const FGameplayTagContainer& NewTags) = 0;
	
	virtual void RemoveTag_NPC(const FGameplayTag& RemovedTag) = 0;
	virtual void RemoveTags_NPC(const FGameplayTagContainer& TagsToRemove) = 0;

	FTagsChangedEvent_NPC OnTagsChangedEvent_NPC;
};
