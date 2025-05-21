#include "Data/QuestEventTriggers.h"

#include "Data/QuestTypes.h"
#include "Subsystems/QuestSubsystem.h"
#include "Subsystems/WorldStateSubsystem.h"

UQuestEventTriggerProxy* FQuestEventTriggerBase::MakeProxy(const FQuestSystemContext& QuestSystemContext,
                                                            const FDataTableRowHandle& InQuestDTRH,
                                                            const FDataTableRowHandle& InQuestEventDTRH) const
{
	auto Proxy = MakeProxyInternal(QuestSystemContext, InQuestDTRH, InQuestEventDTRH);
	if (!ensure(Proxy))
		return nullptr;
	
	Proxy->Initialize(QuestSystemContext, InQuestDTRH, InQuestEventDTRH, EventOccurenceRequirements);
	
	return Proxy;
}

bool FQuestEventTriggerBase::AreRequirementsFulfilled(const FQuestSystemContext& QuestSystemContext) const
{
	for (const auto& QuestEventTriggerRequirementInstancedStruct : EventOccurenceRequirements)
	{
		auto QuestEventTriggerRequirement = QuestEventTriggerRequirementInstancedStruct.GetPtr<FQuestRequirementBase>();
		if (ensure(QuestEventTriggerRequirement))
		{
			if (!QuestEventTriggerRequirement->IsQuestRequirementMet(QuestSystemContext))
				return false;
		}
	}
	
	return true;
}


UQuestEventTriggerProxy* FQuestEventTriggerBase::MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH) const
{
	return nullptr;
}

UQuestEventTriggerProxy* FQuestEventTriggerCrossLocation::MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
                                                                            const FDataTableRowHandle& InQuestDTRH,
                                                                            const FDataTableRowHandle&
                                                                            InQuestEventDTRH) const
{
	auto Proxy = NewObject<UQuestEventTriggerProxy_CrossLocation>(QuestSystemContext.World.Get());
	Proxy->LocationIdTag = LocationIdTag;
	Proxy->ByCharacterTagId = ByCharacterTagId;
	Proxy->CharacterTagsFilter = CharacterTagsFilter;
	Proxy->bEnter = bEnter;
	
	return Proxy;
}

UQuestEventTriggerProxy* FQuestEventTriggerCharacterInventoryChanged::MakeProxyInternal(
	const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH) const
{
	auto Proxy = NewObject<UQuestEventTriggerProxy_CharacterInventoryChanged>(QuestSystemContext.World.Get());

	Proxy->CharacterId = CharacterId;;
	Proxy->ItemId = ItemId;
	Proxy->ItemFilter = ItemFilter;
	Proxy->RequiredCount = RequiredCount;

	return Proxy;
}

UQuestEventTriggerProxy* FQuestEventTriggerCharacterKilled::MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
                                                                       const FDataTableRowHandle& InQuestDTRH,
                                                                       const FDataTableRowHandle& InQuestEventDTRH) const
{
	auto Proxy = NewObject<UQuestEventTriggerProxy_CharacterKilled>(QuestSystemContext.World.Get());

	Proxy->ByCharacterId = ByCharacterId;
	Proxy->KilledCharacterId = KilledCharacterId;
	Proxy->KilledCharacterTagsFilter = KilledCharacterTagsFilter;
	Proxy->RequiredCount = RequiredCount;

	return Proxy;
}

UQuestEventTriggerProxy* FQuestEventTriggerInteraction::MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
                                                                   const FDataTableRowHandle& InQuestDTRH,
                                                                   const FDataTableRowHandle& InQuestEventDTRH) const
{
	auto Proxy = NewObject<UQuestEventTriggerProxy_Interaction>(QuestSystemContext.World.Get());

	Proxy->ByCharacterId = ByCharacterId;
	Proxy->InteractionActorId = InteractionActorId;
	Proxy->InteractionActionId = InteractionActionsIds;
	Proxy->InteractionActorTagsFilter = InteractionActorTagsFilter;

	return Proxy;
}

UQuestEventTriggerProxy* FQuestEventTriggerCoveredByWorldState::MakeProxyInternal(
	const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH,
	const FDataTableRowHandle& InQuestEventDTRH) const
{
	auto Proxy = NewObject<UQuestEventTriggerProxy_CoveredByWorldState>(QuestSystemContext.World.Get());
	Proxy->CoveredByWorldState = CoveredByWorldState;
	return Proxy;
}

UQuestEventTriggerProxy* FQuestEventTriggerDialogueLineHeard::MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
                                                                         const FDataTableRowHandle& InQuestDTRH,
                                                                         const FDataTableRowHandle& InQuestEventDTRH) const
{
	auto Proxy = NewObject<UQuestEventTriggerProxy_DialogueLineHeard>(QuestSystemContext.World.Get());

	Proxy->ByCharacterId = ByCharacterId;
	Proxy->PhraseId = PhraseId;
	
	return Proxy;
}

UQuestEventTriggerProxy* FQuestEventTriggerOnCharacterKnockdown::MakeProxyInternal(
	const FQuestSystemContext& QuestSystemContext,
	const FDataTableRowHandle& InQuestDTRH, const FDataTableRowHandle& InQuestEventDTRH) const
{
	auto Proxy = NewObject<UQuestEventTriggerProxy_OnCharacterKnockdown>(QuestSystemContext.World.Get());

	Proxy->KnockdownedCharacterId = KnockdownedCharacterId;
	Proxy->KnockdownedByCharacterId = KnockdownedByCharacterId;
	
	return Proxy;
}

UQuestEventTriggerProxy* FQuestEventTriggerOnNpcGoalCompleted::MakeProxyInternal(const FQuestSystemContext& QuestSystemContext,
                                                                          const FDataTableRowHandle& InQuestDTRH,
                                                                          const FDataTableRowHandle& InQuestEventDTRH) const
{
	auto Proxy = NewObject<UQuestEventTriggerProxy_OnNpcGoalCompleted>(QuestSystemContext.World.Get());

	Proxy->NpcId = NpcId;
	Proxy->NpcCharacterTagsFilter = NpcCharacterTagsFilter;
	Proxy->NpcGoalTagsFilter = NpcGoalTagsFilter;
	
	return Proxy;
}
