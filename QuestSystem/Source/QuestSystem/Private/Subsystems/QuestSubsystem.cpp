#include "Subsystems/QuestSubsystem.h"

#include "FlowSubsystem.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameMode.h"
#include "Subsystems/QuestNpcSubsystem.h"
#include "Subsystems/WorldLocationsSubsystem.h"
#include "Subsystems/WorldStateSubsystem.h"
#include "Interfaces/QuestCharacter.h"
#include "Interfaces/QuestSystemGameMode.h"

UQuestSubsystem* UQuestSubsystem::Get(const UObject* WorldContextObject)
{
	return WorldContextObject ? WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UQuestSubsystem>() : nullptr;
}

void UQuestSubsystem::RegisterPlayerCharacter(ACharacter* InPlayerCharacter)
{
	PlayerCharacter.SetObject(InPlayerCharacter);
	PlayerCharacter.SetInterface(Cast<IQuestCharacter>(InPlayerCharacter));
}

void UQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Reset();
}

void UQuestSubsystem::OnItemAcquired(IQuestCharacter* QuestCharacter, const FGameplayTag& ItemTagId, const FGameplayTagContainer& ItemTags, int Count)
{
	QuestCharacterAcquiredItemEvent.Broadcast(QuestCharacter, ItemTagId, ItemTags, Count);
}

void UQuestSubsystem::OnLocationReached(const FGameplayTag& LocationIdTag, const IQuestCharacter* EnteredQuestCharacter)
{
	QuestCharacterReachedLocationEvent.Broadcast(LocationIdTag, EnteredQuestCharacter);
}

void UQuestSubsystem::OnLocationLeft(const FGameplayTag& LocationIdTag, const IQuestCharacter* EnteredQuestCharacter)
{
	QuestCharacterLeftLocationEvent.Broadcast(LocationIdTag, EnteredQuestCharacter);
}

void UQuestSubsystem::OnNpcKilled(IQuestCharacter* Killer, IQuestCharacter* Killed)
{
	QuestCharacterKilledEvent.Broadcast(Killer, Killed);
}

void UQuestSubsystem::OnNpcGoalCompleted(IQuestCharacter* Npc, const FGameplayTagContainer& GoalTags, const FGameplayTagContainer& GoalExecutionTags)
{
	QuestNpcGoalCompletedEvent.Broadcast(Npc, GoalTags);
}

void UQuestSubsystem::OnActorInteracted(IQuestCharacter* Interactor, const FGameplayTag& InteractionActorId, const FGameplayTagContainer& InteractionActionsId,
                                        const FGameplayTagContainer& InteractionActorTags)
{
	QuestCharacterInteractedWithItemEvent.Broadcast(Interactor, InteractionActorId, InteractionActionsId, InteractionActorTags);
}

void UQuestSubsystem::OnPlayerHeard(const FGameplayTag& NpcIdTag, const FGameplayTag& NpcPhraseId)
{
	QuestDialogueLineHeardEvent.Broadcast(NpcIdTag, NpcPhraseId);
}

void UQuestSubsystem::OnPlayerDied()
{
	PlayerDiedEvent.Broadcast();
}

bool UQuestSubsystem::StartQuest(const FDataTableRowHandle& QuestDTRH)
{
	if (!CanStartQuest(QuestDTRH))
		return false;

	FQuestDTR* QuestDTR = QuestDTRH.DataTable->FindRow<FQuestDTR>(QuestDTRH.RowName, "");
	bool bInitialized = InitializeQuest(QuestDTRH, QuestDTR);
	if (bInitialized)
		QuestStartedEvent.Broadcast(QuestDTR);
	
	return bInitialized;
}

bool UQuestSubsystem::InitializeQuest(const FDataTableRowHandle& QuestDTRH, const FQuestDTR* QuestDTR)
{
	if (QuestDTR->QuestFlow.IsNull())
		return ensure(false);
	
	FQuestProgress QuestProgress;
	QuestProgress.QuestDTRH = QuestDTRH;

	FlowAssetToQuestIdLookup.Add(QuestDTR->QuestFlow, QuestDTRH.RowName);
	auto FlowSubsystem = GetGameInstance()->GetSubsystem<UFlowSubsystem>();
	auto CreatedQuestFlow = FlowSubsystem->CreateRootFlow(this, QuestDTR->QuestFlow.LoadSynchronous(), false);
	if (!ensure(CreatedQuestFlow))
		return false;
	
	QuestProgress.FlowInstance = CreatedQuestFlow;
	ActiveQuests.Add(QuestDTRH.RowName, QuestProgress);
	CreatedQuestFlow->StartFlow();
	return true;
}

bool UQuestSubsystem::CanStartQuest(const FDataTableRowHandle& QuestDTRH)
{
	if (ActiveQuests.Contains(QuestDTRH.RowName) || CompletedQuests.Contains(QuestDTRH.RowName))
		return false;

	if (!ensure(QuestDTRH.DataTable))
		return false;
	
	FQuestDTR* QuestDTR = QuestDTRH.DataTable->FindRow<FQuestDTR>(QuestDTRH.RowName, "");
	if (QuestDTR == nullptr)
		return false;

	FQuestSystemContext QuestSystemContext = GetQuestSystemContext();
	
	for (const auto& QuestRequirementInstancedStruct : QuestDTR->QuestRequirements)
	{
		if (auto QuestRequirement = QuestRequirementInstancedStruct.GetPtr<FQuestRequirementBase>(); ensure(QuestRequirement))
		{
			if (QuestRequirement->IsApplicable(QuestSystemContext) && !QuestRequirement->IsQuestRequirementMet(QuestSystemContext))
				return false;
		}
	}
	
	return true;
}

void UQuestSubsystem::CompleteQuest(FQuestProgress& CompletedQuest, EQuestState QuestFinalState)
{
	FQuestDTR* QuestDTR = CompletedQuest.QuestDTRH.DataTable->FindRow<FQuestDTR>(CompletedQuest.QuestDTRH.RowName, "");
	if (!ensure(QuestDTR))
		return;
		
	bool bQuestAutocompleted = QuestFinalState == EQuestState::Completed || QuestFinalState == EQuestState::Failed;
		
	CompletedQuests.Add(CompletedQuest.QuestDTRH.RowName, MoveTemp(CompletedQuest));
	ActiveQuests.Remove(CompletedQuest.QuestDTRH.RowName);
	
	if (auto FS = GetGameInstance()->GetSubsystem<UFlowSubsystem>())
		FS->FinishRootFlow(this, QuestDTR->QuestFlow.LoadSynchronous(), EFlowFinishPolicy::Keep);
	
	if (QuestCompletedEvent.IsBound())
		QuestCompletedEvent.Broadcast(QuestDTR, bQuestAutocompleted);
}

void UQuestSubsystem::Load()
{
	Reset();
	auto WSS = UWorldStateSubsystem::Get(this);
	if (WSS)
		WSS->Load();
}

void UQuestSubsystem::Reset()
{
	QuestStartedEvent.Clear();
	QuestCompletedEvent.Clear();
	QuestCharacterReachedLocationEvent.Clear();
	QuestCharacterLeftLocationEvent.Clear();
	QuestCharacterKilledEvent.Clear();
	QuestCharacterAcquiredItemEvent.Clear();
	QuestCharacterInteractedWithItemEvent.Clear();
	QuestDialogueLineHeardEvent.Clear();
	QuestCharacterKnockdownedEvent.Clear();
	QuestNpcGoalCompletedEvent.Clear();
	PlayerDiedEvent.Clear();
	
	FlowAssetToQuestIdLookup.Reset();
	DelayedQuestActions.Reset();
	if (auto FS = GetGameInstance()->GetSubsystem<UFlowSubsystem>())
		for (const auto& ActiveQuest : ActiveQuests)
			if (auto QuestDTR = ActiveQuest.Value.QuestDTRH.GetRow<FQuestDTR>(""))
				if (!QuestDTR->QuestFlow.IsNull())
					FS->FinishRootFlow(this, QuestDTR->QuestFlow.LoadSynchronous(), EFlowFinishPolicy::Abort);
	
	ActiveQuests.Empty();
	CompletedQuests.Empty();
	bStateLoaded = false;
}

void UQuestSubsystem::DelayAction(const FGuid& ActionId, const TScriptInterface<IDelayedQuestAction>& DelayedAction, float GameTimeDelayHours)
{
	DelayedQuestActions.Add(ActionId, DelayedAction);
	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.GameMode->RequestDelayedQuestAction(ActionId, GameTimeDelayHours);
}

void UQuestSubsystem::DelayAction(const FGuid& ActionId, const TScriptInterface<IDelayedQuestAction>& DelayedAction, const FGameplayTag& AtNextDayTime)
{
	DelayedQuestActions.Add(ActionId, DelayedAction);
	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.GameMode->RequestDelayedQuestAction(ActionId, AtNextDayTime);
}

const FName* UQuestSubsystem::GetFlowQuestId(const UFlowAsset* FlowAsset)
{
	return FlowAssetToQuestIdLookup.Find(FlowAsset);
}

void UQuestSubsystem::CompleteFlowQuest(const FName& QuestId, EQuestState QuestFinalState)
{
	if (ensure(ActiveQuests.Contains(QuestId)))
		CompleteQuest(ActiveQuests[QuestId], QuestFinalState);
}

void UQuestSubsystem::AddJournalLog(const FName& QuestId, const FText& JournalEntry,
                                    const FGameplayTagContainer& JournalEntryTags)
{
	// 08.06.2025 @AK: TODO utilize JournalEntryTags 
	auto QuestProgress = ActiveQuests.Find(QuestId);
	if (ensure(QuestProgress))
		QuestProgress->JournalLogs.Emplace(JournalEntry);

	GetQuestSystemContext().GameMode->OnNewJournalLog();
}

void UQuestSubsystem::ExecuteDelayedAction(const FGuid& DelayedActionId)
{
	auto DelayedAction = DelayedQuestActions.Find(DelayedActionId);
	if (ensure(DelayedAction))
	{
		auto QuestSystemContext = GetQuestSystemContext();
		(*DelayedAction)->StartDelayedAction(QuestSystemContext);
		DelayedQuestActions.Remove(DelayedActionId);
	}
}

FVector UQuestSubsystem::GetRandomNavigableLocationNearPlayer(const FVector& PlayerLocation, float Radius, float FloorOffset) const
{
	FVector OutLocation;
	bool bLocationFound = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), PlayerLocation,
		OutLocation, Radius);

	return (bLocationFound ? OutLocation : PlayerLocation) + FVector::UpVector * FloorOffset;
}

void UQuestSubsystem::OnNpcKnockdowned(IQuestCharacter* KnockdownedBy, IQuestCharacter* KnockdownedCharacter)
{
	if (KnockdownedBy && KnockdownedCharacter)
		QuestCharacterKnockdownedEvent.Broadcast(KnockdownedBy, KnockdownedCharacter);
}

void UQuestSubsystem::CompleteQuestEventExternal(const FName& QuestId, const FGuid& QuestFlowNodeId, const FName& FlowNodeOutput)
{
	auto ActiveQuestPtr = ActiveQuests.Find(QuestId);
	if (ActiveQuestPtr == nullptr || !ActiveQuestPtr->FlowInstance.IsValid())
		return;
	
	
	auto FlowNode = ActiveQuestPtr->FlowInstance->GetNode(QuestFlowNodeId);
	if (!ensure(FlowNode != nullptr))
		return;
	
	FlowNode->TriggerOutput(FlowNodeOutput, true);
}

FQuestSystemContext UQuestSubsystem::GetQuestSystemContext()
{
	return FQuestSystemContext
	{
		PlayerCharacter,
		UWorldStateSubsystem::Get(PlayerCharacter.GetObject()),
		UWorldLocationsSubsystem::Get(PlayerCharacter.GetObject()),
		UQuestNpcSubsystem::Get(PlayerCharacter.GetObject()),
		this,
		PlayerCharacter.GetObject()->GetWorld(),
		Cast<IQuestSystemGameMode>(PlayerCharacter.GetObject()->GetWorld()->GetAuthGameMode())
	};
}

FQuestNavigationGuidance UQuestSubsystem::GetNavigationGuidance(const FName& RowName)
{
	FQuestNavigationGuidance Result;
	
	for (const auto& ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest.Value.QuestDTRH.RowName == RowName)
		{
			auto WSS = UWorldStateSubsystem::Get(this);
			auto WLS = GetWorld()->GetSubsystem<UWorldLocationsSubsystem>();

			auto QuestDTR = ActiveQuest.Value.QuestDTRH.GetRow<FQuestDTR>("");
			for (const auto& NavigationGuidance : QuestDTR->NavigationGuideances)
			{
				if (WSS->IsAtWorldState(NavigationGuidance.AtWorldState))
				{
					Result.QuestLocation = WLS->GetClosestQuestLocationSimple(NavigationGuidance.LocationIdTag, PlayerCharacter->GetCharacterLocation());
					Result.UntilWorldState = NavigationGuidance.UntilWorldState;
					return Result;
				}	
			}
		}
	}

	return Result;
}

const UWorldLocationComponent* UQuestSubsystem::GetQuestLocation(const FGameplayTag& LocationIdTag, const FVector& QuerierLocation) const
{
	auto WLS = GetWorld()->GetSubsystem<UWorldLocationsSubsystem>();
	if (!WLS)
		return nullptr;

	return WLS->GetClosestQuestLocationSimple(LocationIdTag, QuerierLocation);
}
