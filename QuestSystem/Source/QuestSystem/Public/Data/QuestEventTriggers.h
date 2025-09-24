#pragma once

#include "InstancedStruct.h"
#include "Engine/DataTable.h"
#include "QuestRequirements.h"
#include "QuestTypes.h"
#include "QuestEventTriggers.generated.h"

class UQuestEventTriggerProxy;

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual ~FQuestEventTriggerBase() = default;

	// This one is just game designers to give quick description of what this event trigger is about
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description;
	
	UQuestEventTriggerProxy* MakeProxy(const FQuestSystemContext& QuestSystemContext,
	                                              const FDataTableRowHandle& InQuestDTRH,
	                                              const FDataTableRowHandle& InQuestEventDTRH) const;

	// Need it in both instanced struct AND proxy object
	bool AreRequirementsFulfilled(const FQuestSystemContext& QuestSystemContext) const;

	// Whenever something happens that sattisfies this triggers, the trigger will check if the event should be considered 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FQuestRequirementBase>> EventOccurenceRequirements;


protected:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
												  const FDataTableRowHandle& InQuestDTRH,
												  const FDataTableRowHandle& InQuestEventDTRH) const;
};

USTRUCT()
struct FQuestEventTriggerCrossLocation : public FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	                                            const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle&
	                                            InQuestEventDTRH) const override;

	// if true, event will be triggered when a character enters the location, if false - when leaves
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnter = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Location,Location"))
	FGameplayTag LocationIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag ByCharacterTagId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery CharacterTagsFilter;
		
};

USTRUCT()
struct FQuestEventTriggerCharacterInventoryChanged : public FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	                                            const FDataTableRowHandle& InQuestDTRH,
	                                            const FDataTableRowHandle& InQuestEventDTRH) const override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag CharacterId;

	// Can be non-final tag, like Item.Weapon instead of Item.Weapon.Saber.PirateSaber
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Item,Item"))
	FGameplayTag ItemId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ItemFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int RequiredCount = 1;
};

USTRUCT()
struct FQuestEventTriggerCharacterKilled : public FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	                                            const FDataTableRowHandle& InQuestDTRH,
	                                            const FDataTableRowHandle& InQuestEventDTRH) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag ByCharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag KilledCharacterId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery KilledCharacterTagsFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int RequiredCount = 1;
};

USTRUCT()
struct FQuestEventTriggerInteraction : public FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	                                            const FDataTableRowHandle& InQuestDTRH,
	                                            const FDataTableRowHandle& InQuestEventDTRH) const override;
	// Can be none, in this case the event will be triggered disregarding the interactor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag ByCharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Interaction"))
	FGameplayTag InteractionActorId;

	// Can be none, in this case any interaction action will trigger the event. If multiple provided - check is made by "any" operation, not "all". 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Interaction"))
	FGameplayTagContainer InteractionActionsIds;

	// Can be empty
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery InteractionActorTagsFilter;
};

USTRUCT()
struct FQuestEventTriggerCoveredByWorldState : public FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	                                            const FDataTableRowHandle& InQuestDTRH,
	                                            const FDataTableRowHandle& InQuestEventDTRH) const override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery CoveredByWorldState;
};

USTRUCT()
struct FQuestEventTriggerDialogueLineHeard : public FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	                                            const FDataTableRowHandle& InQuestDTRH,
	                                            const FDataTableRowHandle& InQuestEventDTRH) const override;
	// Can be none, in this case the event will be triggered disregarding the character
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag ByCharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Quest,Quest"))
	FGameplayTag PhraseId;
};

USTRUCT()
struct FQuestEventTriggerOnCharacterKnockdown : public FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	                                            const FDataTableRowHandle& InQuestDTRH,
	                                            const FDataTableRowHandle& InQuestEventDTRH) const override;
	// If none - consider player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag KnockdownedCharacterId;

	// Can be null
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer KnockdownedByCharacterId;
};

USTRUCT()
struct FQuestEventTriggerOnNpcGoalCompleted : public FQuestEventTriggerBase
{
	GENERATED_BODY()

public:
	virtual UQuestEventTriggerProxy* MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	                                            const FDataTableRowHandle& InQuestDTRH,
	                                            const FDataTableRowHandle& InQuestEventDTRH) const override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery NpcCharacterTagsFilter;
	
	// Can be null
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery NpcGoalTagsFilter;
};

