#include "Subsystems/QuestSubsystem.h"

#include "FlowSubsystem.h"
#include "GameplayTagAssetInterface.h"
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
	ActiveQuests.Empty();
	CompletedQuests.Empty();
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

void UQuestSubsystem::StartQuest(const FDataTableRowHandle& QuestDTRH)
{
	if (!CanStartQuest(QuestDTRH))
		return;

	FQuestSystemContext QuestSystemContext = GetQuestSystemContext();
	FQuestDTR* QuestDTR = QuestDTRH.DataTable->FindRow<FQuestDTR>(QuestDTRH.RowName, "");
	InitializeQuest(QuestDTRH, QuestDTR);
	
	// 05.08.2025 @AK: this means that the quest uses old quest system version. maybe I should just add versioning to QuestDTR like int QuestSystemVersion
	// or just remove it when i'm sure that FlowGraph is up and running
	if (QuestDTR->QuestEventsDT && QuestDTR->QuestFlow.IsNull())
	{
		const TArray<TInstancedStruct<FQuestActionBase>>& QuestActions = QuestDTR->BeginQuestActions;
		ExecuteQuestActions(QuestSystemContext, QuestActions);
	}
	
	QuestStartedEvent.Broadcast(QuestDTR);
}

void UQuestSubsystem::InitializeQuest(const FDataTableRowHandle& QuestDTRH, const FQuestDTR* QuestDTR)
{
	FQuestProgress QuestProgress;
	QuestProgress.QuestDTRH = QuestDTRH;

	if (!QuestDTR->QuestFlow.IsNull())
	{
		FlowAssetToQuestIdLookup.Add(QuestDTR->QuestFlow, QuestDTRH.RowName);
		ActiveQuests.Add(QuestDTRH.RowName, QuestProgress);
		auto FS = GetGameInstance()->GetSubsystem<UFlowSubsystem>();
		FS->StartRootFlow(this, QuestDTR->QuestFlow.LoadSynchronous(), false);
		return;		
	}

	// @AK 05.08.2025: legacy approach that uses quest events DTRHs. TODO Remove when FlowGraph approach is tested
	TArray<FName> QuestEventsDataTableRowNames = QuestDTR->QuestEventsDT->GetRowNames();
	FQuestSystemContext QuestSystemContext = GetQuestSystemContext();
	
	for (const auto& RowName : QuestEventsDataTableRowNames)
	{
		FDataTableRowHandle QuestEventDTRH;
		QuestEventDTRH.DataTable = QuestDTR->QuestEventsDT;
		QuestEventDTRH.RowName = RowName;
		FQuestEventDTR* QuestEventDTR = QuestEventDTRH.GetRow<FQuestEventDTR>("");
		FQuestEventData QuestEventState;
		QuestEventState.QuestTaskDTRH = QuestEventDTRH;
		QuestEventState.OccuredTrigger = nullptr;
		QuestEventState.CoveredTrigger = nullptr;

		const bool bQuestEventAlreadyCovered = QuestEventDTR->EventCoveredTrigger.IsValid()
			&& QuestEventDTR->EventCoveredTrigger.GetPtr<FQuestEventTriggerBase>()->AreRequirementsFulfilled(QuestSystemContext);
			
		if (!bQuestEventAlreadyCovered)
		{
			if (QuestEventDTR->EventCompletedTrigger.IsValid())
			{
				QuestEventState.OccuredTrigger = QuestEventDTR->EventCompletedTrigger.Get<FQuestEventTriggerBase>().MakeProxy(QuestSystemContext, QuestDTRH, QuestEventDTRH);
				QuestEventState.OccuredTrigger->QuestEventOccuredEvent.BindUObject(this, &UQuestSubsystem::OnQuestEventOccured);
			}

			if (QuestEventDTR->EventCoveredTrigger.IsValid())
			{
				QuestEventState.CoveredTrigger = QuestEventDTR->EventCoveredTrigger.Get<FQuestEventTriggerBase>().MakeProxy(QuestSystemContext, QuestDTRH, QuestEventDTRH);
				QuestEventState.CoveredTrigger->QuestEventOccuredEvent.BindUObject(this, &UQuestSubsystem::OnQuestEventCovered);
			}

			QuestProgress.PendingQuestEvents.Add(RowName, QuestEventState);
		}
		else
		{
			QuestProgress.CoveredQuestEvents.Add(QuestEventDTRH.RowName, QuestEventState);
			if (QuestEventDTR->bExecuteActionsWhenCovered)
				ExecuteQuestActions(QuestSystemContext, QuestEventDTR->EventOccuredActions);
		}
	}
	
	ActiveQuests.Add(QuestDTRH.RowName, QuestProgress);
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

void UQuestSubsystem::OnQuestEventOccured(UQuestEventTriggerProxy* QuestEventTrigger)
{
	CompleteQuestEvent(QuestEventTrigger, false);
}

void UQuestSubsystem::OnQuestEventCovered(UQuestEventTriggerProxy* QuestEventTrigger)
{
	CompleteQuestEvent(QuestEventTrigger, true);
}

void UQuestSubsystem::CompleteQuestEvent(UQuestEventTriggerProxy* QuestEventTrigger, bool bEventCovered)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UQuestSubsystem::CompleteQuestEvent)
	
	// wtf is this situation
	if (!ensure(ActiveQuests.Num() > 0))
		return;
	
	auto ActiveQuest = ActiveQuests.Find(QuestEventTrigger->GetQuestDTRH().RowName);
	if (!ensure(ActiveQuest))
		return;
	
	auto* PendingQuestEvent = ActiveQuest->PendingQuestEvents.Find(QuestEventTrigger->GetQuestEventDTRH().RowName);
	if (!ensure(PendingQuestEvent))
		return;

	QuestEventTrigger->Disable();
	auto OccuredEventDTR = QuestEventTrigger->GetQuestEventDTRH().GetRow<FQuestEventDTR>("");

	if (bEventCovered)
	{
		if (PendingQuestEvent->OccuredTrigger && PendingQuestEvent->OccuredTrigger->IsInitialized())
			PendingQuestEvent->OccuredTrigger->Disable();

		ActiveQuest->CoveredQuestEvents.Add(QuestEventTrigger->GetQuestEventDTRH().RowName, *PendingQuestEvent);
	}
	else
	{
		if (PendingQuestEvent->CoveredTrigger != nullptr && PendingQuestEvent->CoveredTrigger->IsInitialized())
			PendingQuestEvent->CoveredTrigger->Disable();

		ActiveQuest->CompletedQuestEvents.Add(QuestEventTrigger->GetQuestEventDTRH().RowName, *PendingQuestEvent);
		const FQuestDTR* QuestRow = QuestEventTrigger->GetQuestDTRH().GetRow<FQuestDTR>("");
		QuestEventOccuredEvent.Broadcast(QuestRow, OccuredEventDTR);
	}

	PendingQuestEvent->Finalize();
	ActiveQuest->PendingQuestEvents.Remove(QuestEventTrigger->GetQuestEventDTRH().RowName);
		
	bool bNoPendingEventLeft = true;
	if (OccuredEventDTR->QuestStateChange != EQuestState::Completed && OccuredEventDTR->QuestStateChange != EQuestState::Failed)
	{
		for (const auto& PendingQuestTask : ActiveQuest->PendingQuestEvents)
		{
			FQuestEventDTR* QuestTaskDTR = PendingQuestTask.Value.GetQuestEventDTR();
			if (!QuestTaskDTR->bImplicit)
			{
				bNoPendingEventLeft = false;
				break;
			}
		}
	}

	if (!bEventCovered || OccuredEventDTR->bExecuteActionsWhenCovered)
	{
		auto QuestSystemContext = GetQuestSystemContext();
		ExecuteQuestActions(QuestSystemContext, OccuredEventDTR->EventOccuredActions);
	}
		
	if (bNoPendingEventLeft || OccuredEventDTR->QuestStateChange == EQuestState::Completed || OccuredEventDTR->QuestStateChange == EQuestState::Failed)
	{
		EQuestState FinalQuestState = OccuredEventDTR->QuestStateChange == EQuestState::InProgress && bNoPendingEventLeft
			? EQuestState::Completed
			: OccuredEventDTR->QuestStateChange;
		ActiveQuest->QuestState = FinalQuestState;
		CompleteQuest(*ActiveQuest, FinalQuestState);
	}
}

void UQuestSubsystem::CompleteQuest(FQuestProgress& CompletedQuest, EQuestState QuestFinalState)
{
	FQuestSystemContext QuestSystemContext = GetQuestSystemContext();

	FQuestDTR* QuestDTR = CompletedQuest.QuestDTRH.DataTable->FindRow<FQuestDTR>(CompletedQuest.QuestDTRH.RowName, "");
	if (!ensure(QuestDTR))
		return;
		
	bool bQuestAutocompleted = QuestFinalState == EQuestState::Completed || QuestFinalState == EQuestState::Failed;
		
	for (auto& PendingTask : CompletedQuest.PendingQuestEvents)
	{
		if (PendingTask.Value.OccuredTrigger != nullptr && PendingTask.Value.OccuredTrigger->IsInitialized())
			PendingTask.Value.OccuredTrigger->Disable();

		if (PendingTask.Value.CoveredTrigger != nullptr && PendingTask.Value.CoveredTrigger->IsInitialized())
			PendingTask.Value.CoveredTrigger->Disable();

		PendingTask.Value.Finalize();
	}

	if (QuestDTR->QuestEventsDT != nullptr && QuestDTR->QuestFlow.IsNull()) // TODO remove when QuestFlow is tested
		ExecuteQuestActions(QuestSystemContext, QuestFinalState != EQuestState::Failed ? QuestDTR->SuccessfulEndQuestActions : QuestDTR->FailureEndQuestActions);

	CompletedQuests.Add(CompletedQuest.QuestDTRH.RowName, MoveTemp(CompletedQuest));
	ActiveQuests.Remove(CompletedQuest.QuestDTRH.RowName);
	
	if (QuestCompletedEvent.IsBound())
		QuestCompletedEvent.Broadcast(QuestDTR, bQuestAutocompleted);
}

void UQuestSubsystem::Load()
{
	ActiveQuests.Empty();
	CompletedQuests.Empty();
	auto WSS = UWorldStateSubsystem::Get(this);
	if (WSS)
		WSS->Load();
}

void UQuestSubsystem::ExecuteQuestActions(const FQuestSystemContext& QuestSystemContext, const TArray<TInstancedStruct<FQuestActionBase>>& QuestActions)
{
	for (const auto& QuestActionInstancedStruct : QuestActions)
	{
		if (!ensure(QuestActionInstancedStruct.IsValid()))
			continue;

		const auto& QuestAction = QuestActionInstancedStruct.Get<FQuestActionBase>();
		if (!QuestAction.IsEnabled())
			continue;
		
		if (!QuestAction.IsDelayed())
		{
			QuestAction.Execute(QuestSystemContext);
		}
		else // if (QuestAction.CanExecute(QuestSystemContext))
		{
			// 17.12.2024 @AK: deliberately not checking if quest action can be executed now because even if it can't now - it might be able in future
			UQuestActionProxy* QuestActionProxy = NewObject<UQuestActionProxy>( QuestSystemContext.World.Get());
			QuestActionProxy->Initialize(QuestActionInstancedStruct, QuestSystemContext);
			DelayedQuestActions.Add(QuestAction.ActionId, QuestActionProxy);
			if (QuestAction.StartAtNextTimeOfDay.IsValid())
				QuestSystemContext.GameMode->RequestDelayedQuestAction(QuestAction.ActionId, QuestAction.StartAtNextTimeOfDay);
			else
				QuestSystemContext.GameMode->RequestDelayedQuestAction(QuestAction.ActionId, QuestAction.GameTimeDelayHours);
		}
	}
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

void UQuestSubsystem::ExecuteQuestActionsExternal(const TArray<TInstancedStruct<FQuestActionBase>>& QuestActions)
{
	auto QuestSystemContext = GetQuestSystemContext();
	ExecuteQuestActions(QuestSystemContext, QuestActions);
}

void UQuestSubsystem::CompleteQuestEventExternal(const FDataTableRowHandle& QuestEventDTRH)
{
	for (const auto& ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest.Value.QuestDTRH.GetRow<FQuestDTR>("")->QuestEventsDT == QuestEventDTRH.DataTable)
		{
			auto QuestEvent = ActiveQuest.Value.PendingQuestEvents.Find(QuestEventDTRH.RowName);
			CompleteQuestEvent(QuestEvent->OccuredTrigger, false);
			break;
		}
	}
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
