// 


#include "Data/QuestEventTriggerProxy.h"

#include "Data/QuestEventTriggers.h"
#include "Data/QuestRequirements.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestSubsystem.h"
#include "Subsystems/WorldStateSubsystem.h"

void UQuestEventTriggerProxy::Initialize(const FQuestSystemContext& QuestSystemContext,
                                         const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
                                         const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestEventTriggerRequirements)
{
	CachedQuestSystemContext = QuestSystemContext;
	QuestDTRH = InQuestDTRH;
	QuestEventDTRH = InQuestEventDTRH;
	EventOccurenceRequirements = InQuestEventTriggerRequirements;
	bInitialized = true;
}

void UQuestEventTriggerProxy::Disable()
{
	if (QuestSystemEventDelegateHandle.IsValid())
	{
		DisableInternal();
		QuestSystemEventDelegateHandle.Reset();
	}
}

bool UQuestEventTriggerProxy::AreRequirementsFulfilled(const FQuestSystemContext& QuestSystemContext) const
{
	for (const auto& QuestEventTriggerRequirementInstancedStruct : EventOccurenceRequirements)
	{
		auto QuestEventTriggerRequirement = QuestEventTriggerRequirementInstancedStruct.GetPtr<FQuestRequirementBase>();
		if (ensure(QuestEventTriggerRequirement))
			if (!QuestEventTriggerRequirement->IsQuestRequirementMet(QuestSystemContext))
				return false;
	}
	
	return true;
}

void UQuestEventTriggerProxy::TriggerEvent()
{
	QuestEventOccuredEvent.ExecuteIfBound(this);
}

void UQuestEventTriggerProxy_CrossLocation::Initialize(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
	const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements)
{
	Super::Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, InQuestRequirements);
	QuestSystemEventDelegateHandle = bEnter
		? QuestSystemContext.QuestSubsystem->QuestCharacterReachedLocationEvent.AddUObject(this, &UQuestEventTriggerProxy_CrossLocation::OnLocationCrossed)
		: QuestSystemContext.QuestSubsystem->QuestCharacterLeftLocationEvent.AddUObject(this, &UQuestEventTriggerProxy_CrossLocation::OnLocationCrossed);
}

void UQuestEventTriggerProxy_CrossLocation::DisableInternal()
{
	Super::DisableInternal();
	if (bEnter)
		CachedQuestSystemContext.QuestSubsystem->QuestCharacterReachedLocationEvent.Remove(QuestSystemEventDelegateHandle);
	else
		CachedQuestSystemContext.QuestSubsystem->QuestCharacterLeftLocationEvent.Remove(QuestSystemEventDelegateHandle);
}

void UQuestEventTriggerProxy_CrossLocation::OnLocationCrossed(const FGameplayTag& VisitedLocationIdTag,
	const IQuestCharacter* QuestCharacter)
{
	if (!ensure(QuestCharacter != nullptr) || !AreRequirementsFulfilled(CachedQuestSystemContext))
		return;

	if (VisitedLocationIdTag == LocationIdTag && QuestCharacter->GetQuestCharacterIdTag() == ByCharacterTagId)
	{
		if (!CharacterTagsFilter.IsEmpty())
		{
			const FGameplayTagContainer CharacterTags = QuestCharacter->GetQuestCharacterTags();
			if (!CharacterTagsFilter.Matches(CharacterTags))
				return;
		}
		
		TriggerEvent();
	}
}

void UQuestEventTriggerProxy_CharacterInventoryChanged::Initialize(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
	const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements)
{
	Super::Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, InQuestRequirements);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestCharacterAcquiredItemEvent.AddUObject(this, &UQuestEventTriggerProxy_CharacterInventoryChanged::OnItemAcquired);
}

void UQuestEventTriggerProxy_CharacterInventoryChanged::DisableInternal()
{
	Super::DisableInternal();
	CachedQuestSystemContext.QuestSubsystem->QuestCharacterAcquiredItemEvent.Remove(QuestSystemEventDelegateHandle);
}

void UQuestEventTriggerProxy_CharacterInventoryChanged::OnItemAcquired(IQuestCharacter* QuestCharacter,
	const FGameplayTag& AcquiredItemId, const FGameplayTagContainer& ItemTags, int Count)
{
	if (!AreRequirementsFulfilled(CachedQuestSystemContext))
		return;

	bool bEventOccured = AcquiredItemId.MatchesTag(ItemId)
		&& QuestCharacter->GetQuestCharacterIdTag() == CharacterId
		&& (ItemFilter.IsEmpty() || ItemFilter.Matches(ItemTags))
		&& RequiredCount <= 1 || QuestCharacter->GetCountOfItem(AcquiredItemId, ItemFilter) >= RequiredCount;  

	if(bEventOccured)
		TriggerEvent();
}

void UQuestEventTriggerProxy_CharacterKilled::Initialize(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
	const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements)
{
	Super::Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, InQuestRequirements);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestCharacterKilledEvent.AddUObject(this, &UQuestEventTriggerProxy_CharacterKilled::OnCharacterKilled);
}

void UQuestEventTriggerProxy_CharacterKilled::DisableInternal()
{
	Super::DisableInternal();
	CachedQuestSystemContext.QuestSubsystem->QuestCharacterKilledEvent.Remove(QuestSystemEventDelegateHandle);
}

void UQuestEventTriggerProxy_CharacterKilled::OnCharacterKilled(IQuestCharacter* Killer, IQuestCharacter* Killed)
{
	if (!AreRequirementsFulfilled(CachedQuestSystemContext))
		return;
	
	if (Killer->GetQuestCharacterIdTag() == ByCharacterId && Killed->GetQuestCharacterIdTag() == KilledCharacterId)
	{
		if (!KilledCharacterTagsFilter.IsEmpty() && !KilledCharacterTagsFilter.Matches(Killed->GetQuestCharacterTags()))
			return;

		CurrentCount++;
		if (CurrentCount >= RequiredCount)
			TriggerEvent();
	}
}

void UQuestEventTriggerProxy_Interaction::Initialize(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
	const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements)
{
	Super::Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, InQuestRequirements);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestCharacterInteractedWithItemEvent.AddUObject(this,
		&UQuestEventTriggerProxy_Interaction::OnCharacterInteractedWithActor);
}

void UQuestEventTriggerProxy_Interaction::DisableInternal()
{
	Super::DisableInternal();
	CachedQuestSystemContext.QuestSubsystem->QuestCharacterInteractedWithItemEvent.Remove(QuestSystemEventDelegateHandle);
}

void UQuestEventTriggerProxy_Interaction::OnCharacterInteractedWithActor(IQuestCharacter* QuestCharacter,
                                                                         const FGameplayTag& CurrentInteractionActorId, const FGameplayTagContainer& CurrentInteractionActionId,
                                                                         const FGameplayTagContainer& InInteractionActorTags)
{
	if (!AreRequirementsFulfilled(CachedQuestSystemContext))
		return;

	bool bEventOccured = (QuestCharacter->GetQuestCharacterIdTag() == ByCharacterId || !ByCharacterId.IsValid())
		&& CurrentInteractionActorId == InteractionActorId
		&& (CurrentInteractionActionId.HasAny(InteractionActionId) || !InteractionActionId.IsValid())
		&& (InteractionActorTagsFilter.IsEmpty() || InteractionActorTagsFilter.Matches(InInteractionActorTags));
	
	if (bEventOccured)
		TriggerEvent();
}

void UQuestEventTriggerProxy_CoveredByWorldState::Initialize(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
	const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements)
{
	Super::Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, InQuestRequirements);
	QuestSystemEventDelegateHandle = QuestSystemContext.WorldStateSubsystem->WorldStateChangedEvent.AddUObject(this, &UQuestEventTriggerProxy_CoveredByWorldState::OnWorldStateChanged);
}

void UQuestEventTriggerProxy_CoveredByWorldState::DisableInternal()
{
	Super::DisableInternal();
	if (CoveredByWorldState.IsEmpty())
		return;

	CachedQuestSystemContext.WorldStateSubsystem->WorldStateChangedEvent.Remove(QuestSystemEventDelegateHandle);
}

void UQuestEventTriggerProxy_CoveredByWorldState::OnWorldStateChanged(const FGameplayTagContainer& NewWorldState)
{
	if (!AreRequirementsFulfilled(CachedQuestSystemContext))
		return;
	
	if (!CoveredByWorldState.IsEmpty() && CoveredByWorldState.Matches(NewWorldState))
		QuestEventOccuredEvent.ExecuteIfBound(this);
}

void UQuestEventTriggerProxy_DialogueLineHeard::Initialize(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
	const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements)
{
	Super::Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, InQuestRequirements);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestDialogueLineHeardEvent.AddUObject(this, &UQuestEventTriggerProxy_DialogueLineHeard::OnDialogueLineHeard);
}

void UQuestEventTriggerProxy_DialogueLineHeard::DisableInternal()
{
	Super::DisableInternal();
	CachedQuestSystemContext.QuestSubsystem->QuestDialogueLineHeardEvent.Remove(QuestSystemEventDelegateHandle);
}

void UQuestEventTriggerProxy_DialogueLineHeard::OnDialogueLineHeard(const FGameplayTag& NpcId,
	const FGameplayTag& HeardPhraseId)
{
	if (!AreRequirementsFulfilled(CachedQuestSystemContext))
		return;
	
	if ((NpcId == ByCharacterId || !ByCharacterId.IsValid()) && HeardPhraseId == PhraseId)
		TriggerEvent();
}

void UQuestEventTriggerProxy_OnCharacterKnockdown::Initialize(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
	const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements)
{
	Super::Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, InQuestRequirements);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestCharacterKnockdownedEvent.AddUObject(this, &UQuestEventTriggerProxy_OnCharacterKnockdown::OnCharacterKnockdowned);
}

void UQuestEventTriggerProxy_OnCharacterKnockdown::DisableInternal()
{
	Super::DisableInternal();
	CachedQuestSystemContext.QuestSubsystem->QuestCharacterKnockdownedEvent.Remove(QuestSystemEventDelegateHandle);
}

void UQuestEventTriggerProxy_OnCharacterKnockdown::OnCharacterKnockdowned(IQuestCharacter* KnockdownedBy,
	IQuestCharacter* KnockdownedCharacter)
{
	if (!AreRequirementsFulfilled(CachedQuestSystemContext))
		return;

	bool bEventOccured = (KnockdownedByCharacterId.IsEmpty() || KnockdownedByCharacterId.HasTag(KnockdownedBy->GetQuestCharacterIdTag()))
		&& (!KnockdownedCharacterId.IsValid() && KnockdownedCharacter->IsPlayer() || KnockdownedCharacterId == KnockdownedCharacter->GetQuestCharacterIdTag());

	if (bEventOccured)
		TriggerEvent();
}

void UQuestEventTriggerProxy_OnNpcGoalCompleted::Initialize(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH,
	const TArray<TInstancedStruct<FQuestRequirementBase>>& InQuestRequirements)
{
	Super::Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, InQuestRequirements);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestNpcGoalCompletedEvent.AddUObject(this, &UQuestEventTriggerProxy_OnNpcGoalCompleted::OnNpcGoalCompleted);
}

void UQuestEventTriggerProxy_OnNpcGoalCompleted::DisableInternal()
{
	Super::DisableInternal();
	CachedQuestSystemContext.QuestSubsystem->QuestNpcGoalCompletedEvent.Remove(QuestSystemEventDelegateHandle);
}

void UQuestEventTriggerProxy_OnNpcGoalCompleted::OnNpcGoalCompleted(IQuestCharacter* Npc,
	const FGameplayTagContainer& QuestGoalTags)
{
	if (!AreRequirementsFulfilled(CachedQuestSystemContext))
		return;
	
	if (ensure(!NpcGoalTagsFilter.IsEmpty()))
		return;
	
	if (NpcId.IsValid() && Npc->GetQuestCharacterIdTag() != NpcId)
		return;

	if (!NpcCharacterTagsFilter.IsEmpty())
	{
		FGameplayTagContainer NpcTags = Npc->GetQuestCharacterTags();
		if (!NpcCharacterTagsFilter.Matches(NpcTags))
			return;
	}
	
	if (NpcGoalTagsFilter.Matches(QuestGoalTags))
		TriggerEvent();
}
