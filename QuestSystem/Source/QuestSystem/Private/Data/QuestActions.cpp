#include "Data/QuestActions.h"

#include "AbilitySet.h"
#include "EngineUtils.h"
#include "GameplayTagAssetInterface.h"
#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Interfaces/QuestSystemGameMode.h"
#include "Objects/ArbitraryQuestAction.h"
#include "Subsystems/QuestNpcSubsystem.h"
#include "Subsystems/QuestSubsystem.h"
#include "Subsystems/WorldLocationsSubsystem.h"
#include "Subsystems/WorldStateSubsystem.h"

FQuestActionBase::FQuestActionBase()
{
	if (!ActionId.IsValid())
		ActionId = FGuid::NewGuid();
}

void FQuestActionBase::Execute(const FQuestSystemContext& Context) const
{
	if (CanExecute(Context))
		ExecuteInternal(Context);
}

bool FQuestActionBase::CanExecute(const FQuestSystemContext& Context) const
{
	for (const auto& QuestRequirementInstancedStruct : ExecuteActionQuestRequirements)
	{
		const auto* QuestRequirement = QuestRequirementInstancedStruct.GetPtr<FQuestRequirementBase>();
		if (ensure(QuestRequirement))
		{
			if (QuestRequirement->IsApplicable(Context) && !QuestRequirement->IsQuestRequirementMet(Context))
				return false;		
		}
	}
	
	return true;
}

void FQuestActionBase::ExecuteInternal(const FQuestSystemContext& Context) const
{
}

void UQuestActionProxy::Initialize(const TInstancedStruct<FQuestActionBase>& Action, const FQuestSystemContext& Context)
{
	QuestAction = Action;
	CachedQuestSystemContext = Context;
}

void UQuestActionProxy::ExecuteDelayedAction(const FQuestSystemContext& QuestSystemContext)
{
	if (ensure(QuestAction.IsValid()))
		QuestAction.GetPtr()->Execute(CachedQuestSystemContext);
}

void FQuestActionRunNpcBehavior::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	Context.NpcSubsystem->TryRunQuestBehavior(QuestActionNpcRunBehavior, ActionId, Context);
}

void FQuestActionSpawnNpc::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	for (auto i = 0; i < SpawnNpcAndSetBehaviorData.Count; i++)
	{
		auto WorldLocation = Context.WorldLocationsSubsystem->GetClosestQuestLocationSimple(SpawnNpcAndSetBehaviorData.LocationIdTag,
			Context.Player->GetCharacterLocation());

		if (!WorldLocation)
			continue;

		const FVector SpawnLocation = WorldLocation->GetRandomLocationInVolume(100.f);
		TScriptInterface<IQuestNPC> Npc = SpawnNpcAndSetBehaviorData.bSpawnNew
			? Context.GameMode->SpawnQuestNPC(SpawnNpcAndSetBehaviorData.NpcIdTag, SpawnLocation, FGameplayTagContainer::EmptyContainer)
			: Context.NpcSubsystem->FindNpc(SpawnNpcAndSetBehaviorData.NpcIdTag, Context.Player->GetCharacterLocation());

		if (Npc == nullptr && !SpawnNpcAndSetBehaviorData.bSpawnNew)
			Npc = Context.GameMode->SpawnQuestNPC(SpawnNpcAndSetBehaviorData.NpcIdTag, SpawnLocation, FGameplayTagContainer::EmptyContainer);

		if (Npc)
		{
			Npc->TeleportToQuestLocation(SpawnLocation);
			Npc->AddNpcQuestTags(SpawnNpcAndSetBehaviorData.WithTags);
			if (SpawnNpcAndSetBehaviorData.OptionalNpcInitialBehavior.RequestedBehaviorIdTag.IsValid())
				Context.NpcSubsystem->RunQuestBehavior(Npc, SpawnNpcAndSetBehaviorData.OptionalNpcInitialBehavior, ActionId, Context);
		}
	}	
}

void FQuestActionSpawnItem::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	
	FVector PlayerLocation = Context.Player->GetCharacterLocation();
	const UWorldLocationComponent* TargetQuestLocation = Context.QuestSubsystem->GetQuestLocation(SpawnItemData.LocationIdTag, PlayerLocation);
	if (!IsValid(TargetQuestLocation))
		return;

	for (auto i = 0; i < SpawnItemData.Count; i++)
	{
		FVector SpawnLocation = SpawnItemData.bNearPlayer
			? Context.QuestSubsystem->GetRandomNavigableLocationNearPlayer(PlayerLocation, SpawnItemData.NearPlayerRadius, SpawnItemData.FloorOffset)
			: TargetQuestLocation->GetRandomLocationInVolume(SpawnItemData.FloorOffset);
	
		Context.GameMode->SpawnItem(SpawnItemData.ItemId, SpawnItemData.ItemCategory,
			SpawnLocation, FRotator::ZeroRotator, SpawnItemData.WithTags);
	}
}

void FQuestActionUpdateCharacterInventory::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	IQuestCharacter* Character = nullptr;
	if (!CharacterId.IsValid() || CharacterId == Context.Player->GetQuestCharacterIdTag())
	{
		Character = Context.Player.GetInterface();
	}
	else
	{
		auto Npc = Context.NpcSubsystem->FindNpc(CharacterId, Context.Player->GetCharacterLocation());
		if (Npc)
		{
			Character = Cast<IQuestCharacter>(Npc.GetObject());
		}
	}

	if (!ensure(Character))
		return;

	for (const auto& Item : ItemsChange)
	{
		Character->ChangeItemsCount(Item.Key, Item.Value.Count);
	}
}

void FQuestActionDestroyActors::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	
	for (TActorIterator<AActor> ActorIterator(Context.NpcSubsystem->GetWorld(), ActorClass); ActorIterator; ++ActorIterator)
	{
		bool bDestroy = true;
		if (!GameplayTagFilter.IsEmpty())
		{
			if (auto GameplayTagActor = Cast<IGameplayTagAssetInterface>(*ActorIterator))
			{
				FGameplayTagContainer GameplayTagContainer;
				GameplayTagActor->GetOwnedGameplayTags(GameplayTagContainer);
				bDestroy = GameplayTagFilter.Matches(GameplayTagContainer);
			}
			else
				bDestroy = false;
		}

		if (bDestroy)
		{
			if (OverTime > 0.f)
				ActorIterator->SetLifeSpan(OverTime);
			else
				ActorIterator->Destroy();
		}
	}
}

void FQuestActionUpdateWorldState::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	Context.WorldStateSubsystem->ChangeWorldState(WorldStateTagsChange, bAppend);
}

void FQuestActionChangeTagsOnPlayer::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	if (bAdd)
		Context.Player->AddQuestTags(Tags);
	else
		Context.Player->RemoveQuestTags(Tags);
}

void FQuestActionChangeTagsOnNpcs::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	Context.NpcSubsystem->ChangeTagsForNpcs(CharacterId, Tags, bAdd, &CharacterFilter);
}

void FQuestActionSetPlayerState::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	if (bAdd)
		Context.Player->AddQuestState(StateTag, SetByCallerParams);
	else
		Context.Player->RemoveQuestState(StateTag);
	
	FQuestActionBase::ExecuteInternal(Context);
}

void FQuestActionSetNpcState::ExecuteInternal(const FQuestSystemContext& Context) const
{
	FQuestActionBase::ExecuteInternal(Context);
	TArray<TScriptInterface<IQuestNPC>> Npcs = Context.NpcSubsystem->GetNpcs(CharacterId, nullptr);
	const bool bMustCheckFilters = !CharacterFilter.IsEmpty();
	for (const auto& Npc : Npcs)
	{
		auto QuestCharacter = Cast<IQuestCharacter>(Npc.GetObject());
		if (bMustCheckFilters)
		{
			FGameplayTagContainer CharacterTags = QuestCharacter->GetQuestCharacterTags();
			if (CharacterFilter.Matches(CharacterTags))
				continue;
		}

		if (bAdd)
			QuestCharacter->AddQuestState(StateTag, SetByCallerParams);
		else
			QuestCharacter->RemoveQuestState(StateTag);
	}
}

void FQuestActionStartQuests::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	for (const auto& QuestToStart : QuestsToStart)
		Context.QuestSubsystem->StartQuest(QuestToStart);
}

void FQuestActionPlayCutscene::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	ensure(false); // not implemented
}

void FQuestActionGiveXP::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	Context.Player->GiveXP(XPReward);
}

void FQuestActionGrantAbilitySet::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Super::ExecuteInternal(Context);
	Context.Player->GrantQuestSystemAbilitySet(GrantedAbilitySet.LoadSynchronous());
	GrantedAbilitySet.Reset();
}

void FQuestActionSetAttitude::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Context.NpcSubsystem->AddCustomAttitude(SourceCharacterIds, TargetCharacterIds, NewAttitude, GameHoursDuration);
}

void FQuestActionSetAttitudePreset::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Context.NpcSubsystem->SetCustomAttitudePreset(ForCharacterIds, NewAttitudePreset, GameHoursDuration);
}

void FQuestActionShowScreenText::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Context.GameMode->ShowScreenText(Title, SubTitle, ScreenTypeTag, ShowDuration);
}

void FQuestActionArbitraryTagAction::ExecuteInternal(const FQuestSystemContext& Context) const
{
	if (ensure(!ArbitraryQuestActionClass.IsNull()))
	{
		auto ActionClass = ArbitraryQuestActionClass.LoadSynchronous();
		auto ArbitraryQuestActionInstance = NewObject<UArbitraryQuestAction>(Context.Player.GetObject(), ActionClass);
		if (ensure(ArbitraryQuestActionInstance))
		{
			FArbitraryQuestActionContext ArbitraryQuestActionContext { Cast<APawn>(Context.Player.GetObject()) };
			ArbitraryQuestActionInstance->Execute(ArbitraryQuestActionContext, TagParameters, FloatParameters);
		}
	}
}

void FQuestActionNpcDisplayReaction::ExecuteInternal(const FQuestSystemContext& Context) const
{
	if (!ensure(PhraseId.IsValid()))
		return;
	
	auto Npc = Context.NpcSubsystem->FindNpc(CharacterId, Context.Player->GetCharacterLocation());
	if (!ensure(Npc.GetObject() != nullptr))
		return;

	Npc->SayQuestPhrase(PhraseId);
}

void FQuestActionInitiateDialogueWithNpc::ExecuteInternal(const FQuestSystemContext& Context) const
{
	auto Npc = Context.NpcSubsystem->FindNpc(NpcId, Context.Player->GetCharacterLocation());
	if (!ensure(Npc.GetObject() != nullptr))
		return;
	
	Context.Player->StartQuestDialogueWithNpc(Cast<AActor>(Npc.GetObject()), OptionalDialogueId);
}

void FQuestActionNpcStartRealtimeDialogue::ExecuteInternal(const FQuestSystemContext& Context) const
{
	auto DialogueInitiatorNpc = Context.NpcSubsystem->FindNpc(NpcId, Context.Player->GetCharacterLocation());
	if (!ensure(DialogueInitiatorNpc))
		return;
	
	TArray<AActor*> DialogueParticipantActors;
	if (bIncludePlayer)
		DialogueParticipantActors.Add(Cast<AActor>(Context.Player.GetObject()));	

	FVector DialogueInitiatorLocation = DialogueInitiatorNpc->GetQuestNpcLocation();
	for (const auto& DialogueParticipant : DialogueParticipants)
	{
		auto PotentialDialogueParticipants = Context.NpcSubsystem->GetNpcsInRange(DialogueParticipant.ParticipantId,
			DialogueInitiatorLocation, DialogueParticipant.InRange, &DialogueParticipant.ParticipantFilter);
		if (DialogueParticipant.Count < PotentialDialogueParticipants.Num())
		{
			TArray<TTuple<AActor*, double>> NpcsBySquareDistances;
			for (const auto& Npc : PotentialDialogueParticipants)
			{
				const auto DistSq = (DialogueInitiatorLocation - Npc->GetQuestNpcLocation()).SizeSquared();
				NpcsBySquareDistances.Add( { Npc->GetQuestNpcPawn(), DistSq });
			}

			NpcsBySquareDistances.Sort([](const TTuple<AActor*, double>& A, const TTuple<AActor*, double>& B) { return A.Value < B.Value; });
			for (int i = 0; i < DialogueParticipant.Count; i++)
				DialogueParticipantActors.Add(NpcsBySquareDistances[i].Key);
		}
		else
		{
			for (const auto& Npc : PotentialDialogueParticipants)
				DialogueParticipantActors.Add(Cast<AActor>(Npc.GetObject()));
		}
	}
	
	DialogueInitiatorNpc->StartQuestDialogue(DialogueId, DialogueParticipantActors);
}

void FQuestActionSendNpcEvent::ExecuteInternal(const FQuestSystemContext& Context) const
{
	auto Npcs = Context.NpcSubsystem->GetNpcs(NpcId, &NpcFilter);
	for (auto& Npc : Npcs)
		Npc->ReceiveQuestEvent(Event);
}


void FQuestActionSetSublevelState::ExecuteInternal(const FQuestSystemContext& Context) const
{
	Context.GameMode->SetQuestSublevelState(SublevelId, bLoaded, PostponeIfPlayerHasTags);	
}
