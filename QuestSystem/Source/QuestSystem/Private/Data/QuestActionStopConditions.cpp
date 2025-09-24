// 


#include "Data/QuestActionStopConditions.h"

#include "Interfaces/QuestSystemGameMode.h"
#include "Subsystems/QuestNpcSubsystem.h"
#include "Subsystems/WorldStateSubsystem.h"

UNpcQuestBehaviorEndConditionProxyBase* FNpcQuestBehaviorEndConditionBase::MakeProxy(const FGuid& InQuestActionId,
	IQuestNPC* InQuestNPC, const FQuestSystemContext& QuestSystemContext) const
{
	auto Proxy = MakeProxyInternal(QuestSystemContext.Player.GetObject());
	Proxy->Initialize(InQuestActionId, InQuestNPC, QuestSystemContext);
	return Proxy;
}

UNpcQuestBehaviorEndConditionProxyBase* FNpcQuestBehaviorEndCondition_UntilWorldState::MakeProxyInternal(UObject* OwnerObject) const
{
	auto Proxy = NewObject<UNpcQuestBehaviorEndConditionProxy_UntilWorldState>(OwnerObject);
	Proxy->UntilWorldState = UntilWorldState;
	return Proxy;
}

UNpcQuestBehaviorEndConditionProxyBase* FNpcQuestBehaviorEndCondition_UntilCharacterState::MakeProxyInternal(UObject* OwnerObject) const
{
	auto Proxy = NewObject<UNpcQuestBehaviorEndConditionProxy_UntilCharacterState>(OwnerObject);
	Proxy->UntilCharacterState = UntilCharacterState;
	return Proxy;
}

UNpcQuestBehaviorEndConditionProxyBase* FNpcQuestBehaviorEndCondition_GameTimeDuration::MakeProxyInternal(UObject* OwnerObject) const
{
	auto Proxy = NewObject<UNpcQuestBehaviorEndConditionProxy_GameTimeDuration>(OwnerObject);
	Proxy->UntilDayTime = UntilDayTime;
	Proxy->GameTimeDurationHours = GameTimeDurationHours;
	return Proxy;
}

void UNpcQuestBehaviorEndConditionProxyBase::Initialize(const FGuid& InQuestActionId, TWeakInterfacePtr<IQuestNPC> InNpc,
                                                        const FQuestSystemContext& InQuestSystemContext)
{
	QuestActionId = InQuestActionId;
	Npc = InNpc;
	QuestSystemContext = InQuestSystemContext;
}

void UNpcQuestBehaviorEndConditionProxyBase::EndConditionTriggered()
{
	// order is important. first disable - then set bEndConditionCompletedFlag
	Disable();
	bEndConditionCompleted = true;
	QuestSystemContext.NpcSubsystem->OnQuestBehaviorConditionTriggered(Npc, QuestActionId);
}

void UNpcQuestBehaviorEndConditionProxy_UntilWorldState::Initialize(const FGuid& InQuestActionId,
	TWeakInterfacePtr<IQuestNPC> InNpc, const FQuestSystemContext& InQuestSystemContext)
{
	Super::Initialize(InQuestActionId, InNpc, InQuestSystemContext);
	if (!ensure(!UntilWorldState.IsEmpty()))
		return;

	if (InQuestSystemContext.WorldStateSubsystem->IsAtWorldState(UntilWorldState))
	{
		EndConditionTriggered();
		return;
	}

	InQuestSystemContext.WorldStateSubsystem->WorldStateChangedEvent.AddUObject(this, &UNpcQuestBehaviorEndConditionProxy_UntilWorldState::OnWorldStateChanged);
}

void UNpcQuestBehaviorEndConditionProxy_UntilWorldState::Disable()
{
	Super::Disable();
	if (!bEndConditionCompleted)
		QuestSystemContext.WorldStateSubsystem->WorldStateChangedEvent.RemoveAll(this);
}

void UNpcQuestBehaviorEndConditionProxy_UntilWorldState::OnWorldStateChanged(const FGameplayTagContainer& NewWorldState)
{
	if (UntilWorldState.Matches(NewWorldState))
		EndConditionTriggered();
}

void UNpcQuestBehaviorEndConditionProxy_UntilCharacterState::Initialize(const FGuid& InQuestActionId,
                                                                        TWeakInterfacePtr<IQuestNPC> InNpc, const FQuestSystemContext& InQuestSystemContext)
{
	Super::Initialize(InQuestActionId, InNpc, InQuestSystemContext);
	if (!ensure(!UntilCharacterState.IsEmpty()))
		return;

	if (UntilCharacterState.Matches(InNpc->GetQuestNpcOwnedTags()))
	{
		EndConditionTriggered();
		return;
	}
	
	InNpc->OnQuestNpcStateChangedEvent.AddUObject(this, &UNpcQuestBehaviorEndConditionProxy_UntilCharacterState::OnCharacterStateChanged);
}

void UNpcQuestBehaviorEndConditionProxy_UntilCharacterState::Disable()
{
	Super::Disable();
	if (!bEndConditionCompleted)
		Npc->OnQuestNpcStateChangedEvent.RemoveAll(this);
}

void UNpcQuestBehaviorEndConditionProxy_UntilCharacterState::OnCharacterStateChanged(const IQuestNPC* QuestNPC,
                                                                                     const FGameplayTagContainer& NewCharacterState)
{
	if (UntilCharacterState.Matches(NewCharacterState))
		EndConditionTriggered();
}

void UNpcQuestBehaviorEndConditionProxy_GameTimeDuration::Initialize(const FGuid& InQuestActionId,
                                                                     TWeakInterfacePtr<IQuestNPC> InNpc, const FQuestSystemContext& InQuestSystemContext)
{
	Super::Initialize(InQuestActionId, InNpc, InQuestSystemContext);
	if (UntilDayTime.IsValid())
		InQuestSystemContext.GameMode->RequestDelayedQuestAction(InQuestActionId, UntilDayTime);
	else
		InQuestSystemContext.GameMode->RequestDelayedQuestAction(InQuestActionId, GameTimeDurationHours);
}

void UNpcQuestBehaviorEndConditionProxy_GameTimeDuration::Disable()
{
	Super::Disable();
	if (!bEndConditionCompleted)
		QuestSystemContext.GameMode->CancelDelayedQuestActionRequest(QuestActionId);
}

void UNpcQuestBehaviorEndConditionProxy_GameTimeDuration::StartDelayedAction(const FQuestSystemContext& InQuestSystemContext)
{
	EndConditionTriggered();
}
