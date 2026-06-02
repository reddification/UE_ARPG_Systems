#pragma once

#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "QuestActions.generated.h"

struct FNpcQuestBehaviorEndConditionBase;

USTRUCT(BlueprintType)
struct FQuestSpawnBaseAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bNearPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "bNearPlayer == true"))
	float NearPlayerRadius = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Location.Id,Location.Id", EditCondition = "bNearPlayer == false"))
	FGameplayTag LocationIdTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer WithTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FloorOffset = 90.f;
};

USTRUCT(BlueprintType)
struct FNpcQuestBehaviorDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Npc.Activity,Npc.Activity"))
	FGameplayTag RequestedBehaviorIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FNpcQuestBehaviorEndConditionBase>> QuestBehaviorEndConditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAnyEndConditionIsEnough = true;	
};

USTRUCT(BlueprintType)
struct FQuestActionSpawnNpcAndSetBehavior : public FQuestSpawnBaseAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag NpcIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSpawnNew = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcQuestBehaviorDescriptor OptionalNpcInitialBehavior;
};

USTRUCT(BlueprintType)
struct FQuestActionNpcRunBehavior
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer NpcIdsTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bFirstOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bPickNpcClosestToPlayer = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcQuestBehaviorDescriptor NpcQuestBehaviorDescriptor;
};

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FItemChangeData
{
	GENERATED_BODY()

	// Can be negative. In this case amount will be subtracted from inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Count = 1;
};

USTRUCT(BlueprintType)
struct FQuestActionSpawnItems : public FQuestSpawnBaseAction
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Item,Item"))
	FGameplayTag ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Item,Item"))
	FGameplayTag ItemCategory;
};
