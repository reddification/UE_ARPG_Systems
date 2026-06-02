#include "Components/Controller/NpcMemoryComponent.h"

#include "GameplayTagContainer.h"
#include "Data/LogChannels.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcActorTagsInterface.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/NpcSystemGameMode.h"

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

void UNpcMemoryComponent::RememberLongTerm(AActor* Actor, const FCharacterPerceptionData& CharacterPerception, const FGuid& Id)
{
	for (const auto& Reason : LongTermMemoryReasons)
	{
		if (!Reason.NpcStateFilter.Matches(OwnerNpcTags))
			continue;
        
		if (!Reason.ActorFilter.Matches(CharacterPerception.CharacterTags))
			continue;

		auto LTM = LongTermMemory.FindOrAdd(Id);
		if (Reason.ForDurationGTH.IsSet())
		{
			const float GTH = Reason.ForDurationGTH.GetValue();
			LTM.RememberedTraits.AddTemporalTraits(Reason.RememberedTraits, GetDateTime(GTH));
			UE_VLOG(OwnerPawn, LogARPGAI_Perception, Verbose, TEXT("Remembered trait %s for %s for %.2f GTH"), 
				*Reason.RememberedTraits.ToStringSimple(), *Actor->GetName(), GTH);
		}
		else
		{
			LTM.RememberedTraits.AddPersistentTraits(Reason.RememberedTraits);
			UE_VLOG(OwnerPawn, LogARPGAI_Perception, Verbose, TEXT("Remembered trait %s for %s forever"), 
				*Reason.RememberedTraits.ToStringSimple(), *Actor->GetName());
		}
	}
    
	if (LongTermMemory.Contains(Id))
	{
		auto& LTM = LongTermMemory[Id];
		LTM.LastUpdateTime = GetWorld()->GetTimeSeconds();
		LTM.Attitude = CharacterPerception.Attitude;
		LTM.bAlive = CharacterPerception.bAlive;
		LTM.LastDetectionSource = CharacterPerception.DetectionSource;
		LTM.LastSeenLocation = Actor->GetActorLocation();
	}
}

void UNpcMemoryComponent::CleanupStaleTraitsMemory()
{
	bool bNoMoreTemporalTraitsMemories = true;
	for (auto& ActorTraitsMemories : LongTermMemory)
	{
		int RemainingTemporalTraits = ActorTraitsMemories.Value.RememberedTraits.CleanupStaleTraits(GetDateTime(0.f));
		bNoMoreTemporalTraitsMemories &= RemainingTemporalTraits == 0;
	}
	
	if (bNoMoreTemporalTraitsMemories)
		GetWorld()->GetTimerManager().ClearTimer(CleanUpRememberedTraitsTimer);
}

void UNpcMemoryComponent::RememberActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits)
{
	auto NpcAliveCreature = Cast<INpcAliveCreature>(Actor);
	if (!NpcAliveCreature)
		return;
	
	auto& TraitsMemory = LongTermMemory.FindOrAdd(NpcAliveCreature->GetId_NpcAliveCreature());
	TraitsMemory.RememberedTraits.AddPersistentTraits(Traits);
}

void UNpcMemoryComponent::RememberActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits, float ForDurationGTH)
{
	auto NpcAliveCreature = Cast<INpcAliveCreature>(Actor);
	if (!NpcAliveCreature)
		return;
	
	auto& Memory = LongTermMemory.FindOrAdd(NpcAliveCreature->GetId_NpcAliveCreature());
	Memory.RememberedTraits.AddTemporalTraits(Traits, GetDateTime(ForDurationGTH));
	if (!CleanUpRememberedTraitsTimer.IsValid())
	{
	    GetWorld()->GetTimerManager().SetTimer(CleanUpRememberedTraitsTimer, 
	        this, &UNpcMemoryComponent::CleanupStaleTraitsMemory, 5.f, true);
	}
}

void UNpcMemoryComponent::ForgetActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits)
{
	auto NpcAliveCreature = Cast<INpcAliveCreature>(Actor);
	if (!NpcAliveCreature)
		return;
	
	auto Id = NpcAliveCreature->GetId_NpcAliveCreature();
	if (auto* Memory = LongTermMemory.Find(Id))
		Memory->RememberedTraits.RemoveTraits(Traits);
}

FGameplayTagContainer UNpcMemoryComponent::GetRememberedActorTraits(const AActor* Actor) const
{
	auto NpcAliveCreature = Cast<INpcAliveCreature>(Actor);
	if (!NpcAliveCreature)
		return FGameplayTagContainer::EmptyContainer;

	if (const auto* Memory = LongTermMemory.Find(NpcAliveCreature->GetId_NpcAliveCreature()))
		return Memory->RememberedTraits.GetTraits();
	
	return FGameplayTagContainer::EmptyContainer;
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
	TemporalTraits.Add(FTemporalTraits { Traits, UntilGameTime });
}

void UNpcMemoryComponent::FTraitsMemory::AddPersistentTraits(const FGameplayTagContainer& Traits)
{
	PersistentTraits.AppendTags(Traits);
}

void UNpcMemoryComponent::FTraitsMemory::RemoveTraits(const FGameplayTagContainer& Traits)
{
	PersistentTraits.RemoveTags(Traits);
	for (int i = TemporalTraits.Num() - 1; i >= 0; --i)
	{
		TemporalTraits[i].Traits.RemoveTags(Traits);
		if (TemporalTraits[i].Traits.IsEmpty())
			TemporalTraits.RemoveAt(i);
	}
}

FGameplayTagContainer UNpcMemoryComponent::FTraitsMemory::GetTraits() const
{
	FGameplayTagContainer Result = PersistentTraits;
	for (const auto& TemporalTraitsIt : TemporalTraits)
		Result.AppendTags(TemporalTraitsIt.Traits);
	
	return Result;
}

int UNpcMemoryComponent::FTraitsMemory::CleanupStaleTraits(const FDateTime& GameTimeNow)
{
	int RemainingTemporalTraitsCount = 0;
	for (int i = TemporalTraits.Num() - 1; i >= 0; --i)
	{
		if (TemporalTraits[i].UntilGameTime <= GameTimeNow)
			TemporalTraits.RemoveAt(i);
		else 
			RemainingTemporalTraitsCount++;
	}
	
	return RemainingTemporalTraitsCount;
}

FDateTime UNpcMemoryComponent::GetDateTime(float ForDurationGTH) const
{
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	return NpcGameMode->GetARPGAIGameTime() + FTimespan::FromHours(ForDurationGTH);
}