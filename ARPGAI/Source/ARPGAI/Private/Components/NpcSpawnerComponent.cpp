#include "Components/NpcSpawnerComponent.h"

#include "Algo/RandomShuffle.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Gameframework/GameModeBase.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Interfaces/NpcZone.h"
#include "Subsystems/NpcSquadSubsystem.h"

UNpcSpawnerComponent::UNpcSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// Called when the game starts
void UNpcSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();
	bool bNeedToSpawn = bSpawnImmediately && !bSpawnedOnStart;
	SetComponentTickEnabled(bNeedToSpawn);
	if (bNeedToSpawn)
	{
		TriggerSpawn();
		bSpawnedOnStart = true;
	}
}

void UNpcSpawnerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SpawnNpcs();
}

void UNpcSpawnerComponent::AddNpc(AActor* Npc)
{
	AliveNpcs.Add(Npc);
	auto NpcAliveCreature = Cast<INpcAliveCreature>(Npc);
	if (ensure(NpcAliveCreature))
		NpcAliveCreature->OnDeathStarted.AddUObject(this, &UNpcSpawnerComponent::OnNpcDied);
}

void UNpcSpawnerComponent::OnNpcDied(AActor* Actor)
{
	auto NpcAliveCreature = Cast<INpcAliveCreature>(Actor);
	NpcAliveCreature->OnDeathStarted.RemoveAll(this);
	AliveNpcs.Remove(Actor);

	if (bRespawnAfterAllNpcsKilled && AliveNpcs.IsEmpty())
	{
		auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
		NpcGameMode->TriggerNpcSpawnerWithDelay(this, RespawnAfterAllNpcsKilledDelayGameHours);
	}
}

UBlackboardData* UNpcSpawnerComponent::GetBlackboardAsset() const
{
	return !NpcBlackboard.IsNull() ? NpcBlackboard.LoadSynchronous() : nullptr;
}

void UNpcSpawnerComponent::TriggerSpawn()
{
	if (!bSpawnerEnabled)
		return;
	
	if (bDestroyAliveNpcsOnRespawnRequested)
	{
		for (const auto& AliveNpc : AliveNpcs)
			if (AliveNpc.IsValid() && IsValid(AliveNpc.Get()))
				AliveNpc->Destroy();

		AliveNpcs.Reset();
	}

	PendingNpcSpawns = NpcSpawnDescriptors;
	Algo::RandomShuffle(PendingNpcSpawns);
	
	SetComponentTickEnabled(true);
	SpawnNpcs();
}

void UNpcSpawnerComponent::SpawnNpcs()
{
	if (!bSpawnerEnabled)
	{
		SetComponentTickEnabled(false);
		return;
	}
	
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	if (!ensure(NpcGameMode))
		return;

	// going reverse order becaouse it's more convenient, but if order matters we can do Algo::Reverse(PendingNpcSpawns);
	
	int SpawnedCount = 0;
	auto NpcSquadSubsystem = UNpcSquadSubsystem::Get(GetWorld());
	for (int i = PendingNpcSpawns.Num() - 1; i >= 0 && SpawnedCount < MaxSpawnsPerTick; --i, ++SpawnedCount)
	{
		const auto& SpawnDescriptor = PendingNpcSpawns[i];
		FVector SpawnWorldLocation = GetOwner()->GetTransform().TransformPosition(SpawnDescriptor.SpawnRelativeLocation);
		UE_VLOG_LOCATION(this, LogARPGAI, Verbose, SpawnWorldLocation, 1, FColor::Cyan, TEXT("Spawning %s"), *SpawnDescriptor.NpcId.ToString());
		auto SpawnedNpc = NpcGameMode->SpawnNpc(SpawnDescriptor.NpcId, SpawnWorldLocation, SpawnNpcWithTags);
		
		AddNpc(SpawnedNpc);

		if (bRegisterAsDesignatedZoneForNpc)
		{
			auto NpcAreasComponent = SpawnedNpc->FindComponentByClass<UNpcAreasComponent>();
			if (auto OwnerNpcZoneInterface = Cast<INpcZone>(GetOwner()))
			{
				TScriptInterface<INpcZone> NpcZone;
				NpcZone.SetObject(GetOwner());
				NpcZone.SetInterface(OwnerNpcZoneInterface);
				NpcAreasComponent->AddAreaOfInterest(AIGameplayTags::Location_Spawner, NpcZone);
			}
	
			if (!NpcFloatBlackboardParameters.IsEmpty())
			{
				auto NpcComponent = SpawnedNpc->FindComponentByClass<UNpcComponent>();
				auto BlackboardKeys = NpcComponent->GetNpcDTR()->NpcBlackboardDataAsset;
				auto Blackboard = SpawnedNpc->GetController()->FindComponentByClass<UBlackboardComponent>();
				for (const auto& BlackboardParameter : NpcFloatBlackboardParameters)
					if (auto BlackboardAlias = BlackboardKeys->BlackboardKeysAliases.Find(BlackboardParameter.Key))
						Blackboard->SetValueAsFloat(BlackboardAlias->SelectedKeyName, BlackboardParameter.Value);
			}
		}

		if (SpawnDescriptor.SquadId.IsValid())
			NpcSquadSubsystem->JoinSquad(SpawnedNpc, SpawnDescriptor.SquadId);

		PendingNpcSpawns.RemoveAt(i);
	}

	if (PendingNpcSpawns.IsEmpty())
		SetComponentTickEnabled(false);
}
