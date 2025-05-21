// 

#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "QuestTypes.h"
#include "UObject/Object.h"
#include "QuestEventTriggerProxy.generated.h"

struct FQuestRequirementBase;
/**
 * 
 */
UCLASS()
class UQuestEventTriggerProxy : public UObject
{
	GENERATED_BODY()

	friend struct FQuestEventTriggerBase;
	
private:
	DECLARE_DELEGATE_OneParam(FQuestEventOccuredEvent, UQuestEventTriggerProxy* QuestEventTrigger);

public:
	FORCEINLINE bool IsInitialized() const { return bInitialized; }
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestEventTriggerRequirements);

	void Disable();

	mutable FQuestEventOccuredEvent QuestEventOccuredEvent;

	FORCEINLINE const FDataTableRowHandle& GetQuestDTRH() const { return QuestDTRH; }
	FORCEINLINE const FDataTableRowHandle& GetQuestEventDTRH() const { return QuestEventDTRH; }
	
	bool AreRequirementsFulfilled(const FQuestSystemContext& QuestSystemContext) const;

protected:
	FDelegateHandle QuestSystemEventDelegateHandle;

	FQuestSystemContext CachedQuestSystemContext;

	virtual void DisableInternal() {};

	void TriggerEvent();

private:
	FDataTableRowHandle QuestDTRH;
	FDataTableRowHandle QuestEventDTRH;
	bool bInitialized = false;
	TArray<TInstancedStruct<FQuestRequirementBase>> EventOccurenceRequirements;
};

UCLASS()
class UQuestEventTriggerProxy_CrossLocation : public UQuestEventTriggerProxy
{
	GENERATED_BODY()

	friend struct FQuestEventTriggerCrossLocation;
	
public:
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements) override;

protected:
	virtual void DisableInternal() override;
	
private:
	FGameplayTag LocationIdTag;
	FGameplayTag ByCharacterTagId;
	FGameplayTagQuery CharacterTagsFilter;
	bool bEnter = true;

	void OnLocationCrossed(const FGameplayTag& VisitedLocationIdTag, const IQuestCharacter* QuestCharacter);
};

UCLASS()
class UQuestEventTriggerProxy_CharacterInventoryChanged : public UQuestEventTriggerProxy
{
	GENERATED_BODY()

friend struct FQuestEventTriggerCharacterInventoryChanged;
	
public:
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements) override;
	
protected:
	virtual void DisableInternal() override;

private:
	void OnItemAcquired(IQuestCharacter* QuestCharacter, const FGameplayTag& ItemId, const FGameplayTagContainer& ItemTags, int Count);
	
	FGameplayTag CharacterId;
	FGameplayTag ItemId;
	FGameplayTagQuery ItemFilter;
	int RequiredCount = 1;
};

UCLASS()
class UQuestEventTriggerProxy_CharacterKilled : public UQuestEventTriggerProxy
{
	GENERATED_BODY()

	friend struct FQuestEventTriggerCharacterKilled;
	
public:
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements) override;
	
protected:
	virtual void DisableInternal() override;

private:
	void OnCharacterKilled(IQuestCharacter* Killer, IQuestCharacter* Killed);
	
	FGameplayTag ByCharacterId;
	FGameplayTag KilledCharacterId;
	FGameplayTagQuery KilledCharacterTagsFilter;
	int RequiredCount = 1;
	
	int CurrentCount = 0;
};

UCLASS()
class UQuestEventTriggerProxy_Interaction : public UQuestEventTriggerProxy
{
	GENERATED_BODY()

friend struct FQuestEventTriggerInteraction;
	
public:
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements) override;
	
protected:
	virtual void DisableInternal() override;

private:
	void OnCharacterInteractedWithActor(IQuestCharacter* QuestCharacter, const FGameplayTag& InteractionActorId, const FGameplayTagContainer&
	                                    InteractionActionId, const FGameplayTagContainer& InteractionActorTags);
	
	FGameplayTag ByCharacterId;
	FGameplayTag InteractionActorId;
	FGameplayTagContainer InteractionActionId;
	FGameplayTagQuery InteractionActorTagsFilter;
};

UCLASS()
class UQuestEventTriggerProxy_CoveredByWorldState : public UQuestEventTriggerProxy
{
	GENERATED_BODY()

	friend struct FQuestEventTriggerCoveredByWorldState;
	
public:
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements) override;
	
protected:
	virtual void DisableInternal() override;

private:
	void OnWorldStateChanged(const FGameplayTagContainer& NewWorldState);
	FGameplayTagQuery CoveredByWorldState;
};

UCLASS()
class UQuestEventTriggerProxy_DialogueLineHeard : public UQuestEventTriggerProxy
{
	GENERATED_BODY()

friend struct FQuestEventTriggerDialogueLineHeard;
	
public:
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements) override;
	
protected:
	virtual void DisableInternal() override;

private:
	void OnDialogueLineHeard(const FGameplayTag& NpcId, const FGameplayTag& HeardPhraseId);

	FGameplayTag ByCharacterId;
	FGameplayTag PhraseId;
};

UCLASS()
class UQuestEventTriggerProxy_OnCharacterKnockdown : public UQuestEventTriggerProxy
{
	GENERATED_BODY()

friend struct FQuestEventTriggerOnCharacterKnockdown;
	
public:
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements) override;
	
protected:
	virtual void DisableInternal() override;

private:
	void OnCharacterKnockdowned(IQuestCharacter* KnockdownedBy, IQuestCharacter* KnockdownedCharacter);

	FGameplayTag KnockdownedCharacterId;
	FGameplayTagContainer KnockdownedByCharacterId;
};

UCLASS()
class UQuestEventTriggerProxy_OnNpcGoalCompleted : public UQuestEventTriggerProxy
{
	GENERATED_BODY()

friend struct FQuestEventTriggerOnNpcGoalCompleted;
	
public:
	virtual void Initialize(const FQuestSystemContext& QuestSystemContext, const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
		const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements) override;
	
protected:
	virtual void DisableInternal() override;

private:
	void OnNpcGoalCompleted(IQuestCharacter* Npc, const FGameplayTagContainer& QuestGoalTags);

	FGameplayTag NpcId;
	FGameplayTagQuery NpcCharacterTagsFilter;
	FGameplayTagQuery NpcGoalTagsFilter;
};




