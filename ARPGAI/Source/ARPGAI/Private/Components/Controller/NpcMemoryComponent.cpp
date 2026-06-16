#include "Components/Controller/NpcMemoryComponent.h"

#include "AIHelpers.h"
#include "GameplayTagContainer.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcActorTagsInterface.h"
#include "Interfaces/NpcAliveActor.h"

UNpcMemoryComponent::UNpcMemoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false; 
}

void UNpcMemoryComponent::SetPawn(APawn* InPawn)
{
	OwnerPawn = InPawn;
	if (auto Npc = Cast<INpcActorTagsInterface>(InPawn))
	{
		Npc->OnTagsChangedEvent_NPC.AddUObject(this, &UNpcMemoryComponent::OnNpcTagsChanged);
		OwnerNpcTags = Npc->GetTags_NPC();
	}
}

// Called when the game starts
void UNpcMemoryComponent::BeginPlay()
{
	Super::BeginPlay();
	LongTermMemory.Reserve(25);
}

void UNpcMemoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto WorldLocal = GetWorld())
		WorldLocal->GetTimerManager().ClearTimer(CleanUpRememberedTraitsTimer);
	
	Super::EndPlay(EndPlayReason);
}

void UNpcMemoryComponent::RememberLongTerm(AActor* Actor, const FCharacterShortTermMemory& CharacterSTM, const FGuid& Id)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UNpcMemoryComponent::RememberLongTerm)

	if (!ensure(Id.IsValid()))
		return;
	
	for (const auto& Reason : LongTermMemoryReasons)
	{
		if (!Reason.NpcStateFilter.IsEmpty() && !Reason.NpcStateFilter.Matches(OwnerNpcTags))
			continue;

		if (!Reason.AttitudesFilter.IsEmpty() && !CharacterSTM.Attitude.MatchesAny(Reason.AttitudesFilter))
			continue;
		
		FGameplayTagContainer CharacterTagsWithLTM = CharacterSTM.CharacterTags;
		if (auto ExistingLTM = LongTermMemory.Find(Id))
			CharacterTagsWithLTM.AppendTags(ExistingLTM->RememberedTraits.GetTraits());
		
		if (!Reason.ActorFilter.IsEmpty() && !Reason.ActorFilter.Matches(CharacterTagsWithLTM))
			continue;

		auto& LTM = LongTermMemory.FindOrAdd(Id);
		if (Reason.ForDurationGTH.IsSet())
		{
			RememberTemporaryTrait(Actor, LTM, Reason.RememberedTraits, Reason.ForDurationGTH.GetValue());
		}
		else
		{
			LTM.RememberedTraits.AddPersistentTraits(Reason.RememberedTraits);
			UE_VLOG(OwnerPawn, LogARPGAI_Perception, Verbose, TEXT("Remembered trait %s for %s forever"), 
				*Reason.RememberedTraits.ToStringSimple(), *Actor->GetName());
		}
	}
	
	if (auto LTM = LongTermMemory.Find(Id))
	{
		LTM->LastUpdateTime = GetWorld()->GetTimeSeconds();
		LTM->LastUpdateGameTime = GetGameWorldTime(this);
		LTM->Attitude = CharacterSTM.Attitude;
		LTM->bAlive = CharacterSTM.bAlive;
		LTM->LastDetectionSource = CharacterSTM.DetectionSource;
		if (CharacterSTM.HasImmediateVisualDetection())
			LTM->LastSeenLocation = Actor->GetActorLocation();
	}
}

void UNpcMemoryComponent::CleanupStaleTraitsMemory()
{
	bool bNoMoreTemporalTraitsMemories = true;
	FDateTime GameWorldTime = GetGameWorldTime(this);
	TArray<FGuid, TInlineAllocator<2>> PendingDeleteKeys;
	for (auto& ActorTraitsMemories : LongTermMemory)
	{
		ActorTraitsMemories.Value.RememberedTraits.CleanupStaleTraits(GameWorldTime);
		bNoMoreTemporalTraitsMemories &= !ActorTraitsMemories.Value.RememberedTraits.HasTemporalTraits();
		if (!ActorTraitsMemories.Value.RememberedTraits.HasTraits())
			PendingDeleteKeys.Add(ActorTraitsMemories.Key);
	}
	
	for (const auto& PendingDeleteKey : PendingDeleteKeys)
		LongTermMemory.Remove(PendingDeleteKey);
	
	if (bNoMoreTemporalTraitsMemories)
		GetWorld()->GetTimerManager().ClearTimer(CleanUpRememberedTraitsTimer);
}

void UNpcMemoryComponent::RememberActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits)
{
	auto NpcAliveCreature = Cast<INpcAliveActor>(Actor);
	if (!NpcAliveCreature)
		return;
	
	auto& TraitsMemory = LongTermMemory.FindOrAdd(NpcAliveCreature->GetId_NPC());
	TraitsMemory.RememberedTraits.AddPersistentTraits(Traits);
}

void UNpcMemoryComponent::RememberTemporaryTrait(const AActor* ForActor, FLongTermMemoryActorData& Memory, const FGameplayTagContainer& Traits, float DurationGTH)
{
	Memory.RememberedTraits.AddTemporalTraits(Traits, GetGameWorldTime(this, DurationGTH));
	UE_VLOG(OwnerPawn, LogARPGAI_Perception, Verbose, TEXT("Remembered trait %s for %s for %.2f GTH"), 
		*Traits.ToStringSimple(), *ForActor->GetName(), DurationGTH);
	if (!CleanUpRememberedTraitsTimer.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(CleanUpRememberedTraitsTimer, 
			this, &UNpcMemoryComponent::CleanupStaleTraitsMemory, 2.f, true);
	}
}

void UNpcMemoryComponent::RememberActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits, float ForDurationGTH)
{
	auto NpcAliveCreature = Cast<INpcAliveActor>(Actor);
	if (!NpcAliveCreature)
		return;
	
	auto& Memory = LongTermMemory.FindOrAdd(NpcAliveCreature->GetId_NPC());
	RememberTemporaryTrait(Actor, Memory, Traits, ForDurationGTH);
}

void UNpcMemoryComponent::ForgetActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits)
{
	auto NpcAliveCreature = Cast<INpcAliveActor>(Actor);
	if (!NpcAliveCreature)
		return;
	
	auto Id = NpcAliveCreature->GetId_NPC();
	if (auto* Memory = LongTermMemory.Find(Id))
		Memory->RememberedTraits.RemoveTraits(Traits);
}

FGameplayTagContainer UNpcMemoryComponent::GetRememberedActorTraits(const AActor* Actor) const
{
	auto NpcAliveCreature = Cast<INpcAliveActor>(Actor);
	if (!NpcAliveCreature)
		return FGameplayTagContainer::EmptyContainer;

	if (const auto* Memory = LongTermMemory.Find(NpcAliveCreature->GetId_NPC()))
		return Memory->RememberedTraits.GetTraits();
	
	return FGameplayTagContainer::EmptyContainer;
}

int UNpcMemoryComponent::GetAlliesKilledByCount(const AActor* Murderer, float GameTimeHoursThreshold)
{
	if (DeadAlliesHistory.IsEmpty())
		return 0;
	
	auto NpcAliveCreature = Cast<INpcAliveActor>(Murderer);
	if (NpcAliveCreature == nullptr)
		return 0;
	
	FGuid MurdererId = NpcAliveCreature->GetId_NPC();

	FDateTime CurrentDateTime = GetGameWorldTime(this);
	int Result = 0;
	for (const auto& DeadAlly : DeadAlliesHistory)
	{
		if (DeadAlly.MurdererId != MurdererId)
			continue;
		
		if (GameTimeHoursThreshold <= 0.f || DeadAlly.KilledAt + FTimespan::FromHours(GameTimeHoursThreshold) >= CurrentDateTime)
			Result++;
	}
	
	return Result;
}

void UNpcMemoryComponent::RememberAllyDied(APawn* DeadAlly, AActor* Murderer, const FGameplayTag& LastHitType)
{
	FKilledAllyData KilledAllyData;
	KilledAllyData.MurderActor = Murderer;
	KilledAllyData.KilledActor = DeadAlly;
	KilledAllyData.KilledAt = GetGameWorldTime(this);
	KilledAllyData.LastHitTag = LastHitType;
	
	if (auto DeadAllyAliveActorInterface = Cast<INpcAliveActor>(DeadAlly))
		KilledAllyData.KilledActorId = DeadAllyAliveActorInterface->GetId_NPC();
	
	if (auto MurdererAliveActorInterface = Cast<INpcAliveActor>(Murderer))
		KilledAllyData.MurdererId = MurdererAliveActorInterface->GetId_NPC();
	
	DeadAlliesHistory.Add(KilledAllyData);
}

void UNpcMemoryComponent::OnNpcTagsChanged(AActor* Pawn, const FGameplayTagContainer& NewTags)
{
	if (ensure(Pawn == OwnerPawn))
		OwnerNpcTags = NewTags;
}

// ========================= Traits Memory

void UNpcMemoryComponent::FTraitsMemory::AddTemporalTraits(const FGameplayTagContainer& Traits,
	const FDateTime& UntilGameTime)
{
	
	for (const auto& NewTemporalTrait : Traits)
	{
		if (TemporalTraits.Contains(NewTemporalTrait))
		{
			if (TemporalTraits[NewTemporalTrait] < UntilGameTime)
				TemporalTraits[NewTemporalTrait] = UntilGameTime;
		}
		else 
			TemporalTraits.Emplace(NewTemporalTrait, UntilGameTime);
	}
}

void UNpcMemoryComponent::FTraitsMemory::AddPersistentTraits(const FGameplayTagContainer& Traits)
{
	PersistentTraits.AppendTags(Traits);
}

void UNpcMemoryComponent::FTraitsMemory::RemoveTraits(const FGameplayTagContainer& RemovedTraits)
{
	PersistentTraits.RemoveTags(RemovedTraits);
	
	FGameplayTagContainer RemovedTempTraits;
	for (const auto& TemporalTrait : TemporalTraits)
		if (TemporalTrait.Key.MatchesAny(RemovedTraits))
			RemovedTempTraits.AddTagFast(TemporalTrait.Key);
	
	for (const auto& RemovedTempTrait : RemovedTempTraits)
		TemporalTraits.Remove(RemovedTempTrait);
}

FGameplayTagContainer UNpcMemoryComponent::FTraitsMemory::GetTraits() const
{
	FGameplayTagContainer Result = PersistentTraits;
	for (const auto& TemporalTraitsIt : TemporalTraits)
		Result.AddTag(TemporalTraitsIt.Key);
	
	return Result;
}

void UNpcMemoryComponent::FTraitsMemory::CleanupStaleTraits(const FDateTime& GameTimeNow)
{
	auto TempTraitsCopy = TemporalTraits;
	for (const auto& TemporalTraitCopy : TempTraitsCopy)
		if (TemporalTraitCopy.Value <= GameTimeNow)
			TemporalTraits.Remove(TemporalTraitCopy.Key);
}