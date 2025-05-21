#include "Components/NpcSpawnerComponent.h"

#include "Algo/RandomShuffle.h"
#include "Components/BoxComponent.h"
#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"
#include "Gameframework/GameModeBase.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Interfaces/NpcZone.h"

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
	NpcSpawnedThisTick = 0;
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
	
	PendingNpcSpawnIndex = 0;
	SetComponentTickEnabled(true);
	SpawnNpcs();
}

void UNpcSpawnerComponent::ProvideEqsPoints(TArray<FNavLocation>& OutEqsPoints, const float Density,
	const float ExtentScale) const
{
	auto NpcZone = Cast<INpcZone>(GetOwner());
	if (!ensure(NpcZone))
		return;
	
	TArray<UShapeComponent*> ZoneVolumes = INpcZone::Execute_GetZoneCollisionVolumes(GetOwner());

	for (const auto* ZoneVolume : ZoneVolumes)
	{
		// TODO handle other volume types
		const UBoxComponent* BoxComponent = Cast<const UBoxComponent>(ZoneVolume);
		if (!ensure(BoxComponent))
			continue;
		
		FVector GuardZoneExtent = BoxComponent->GetScaledBoxExtent();
		FVector Location = GetOwner()->GetActorLocation();
		FVector ForwardVector = GetOwner()->GetActorForwardVector();
		FVector RightVector = GetOwner()->GetActorRightVector();

		const int32 ItemCountX = GuardZoneExtent.X * 2.f * ExtentScale / Density;
		const int32 ItemCountY = GuardZoneExtent.Y * 2.f * ExtentScale / Density;
	
		for (int32 IndexX = - ItemCountX / 2; IndexX <= ItemCountX / 2; ++IndexX)
		{
			for (int32 IndexY = - ItemCountY / 2; IndexY <= ItemCountY / 2; ++IndexY)
			{
				FVector ItemLocation = Location + ForwardVector * Density * IndexX + RightVector * Density * IndexY;
				const FNavLocation TestPoint = FNavLocation(ItemLocation);
				OutEqsPoints.Add(TestPoint);
			}
		}	
	}
}

void UNpcSpawnerComponent::SpawnNpcs()
{
	if (!bSpawnerEnabled)
	{
		SetComponentTickEnabled(false);
		return;
	}
	
	const int TotalSpawns = FMath::Min(NpcsToSpawnPerActivation, NpcSpawnDescriptors.Num());
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	if (!ensure(NpcGameMode))
		return;

	int SpawnedCount = 0;
	while (NpcSpawnedThisTick < MaxSpawnsPerTick && PendingNpcSpawnIndex + NpcSpawnedThisTick < TotalSpawns )
	{
		const auto& SpawnDescriptor = PendingNpcSpawns[PendingNpcSpawnIndex + NpcSpawnedThisTick];
		FVector SpawnWorldLocation = GetOwner()->GetTransform().TransformPosition(SpawnDescriptor.SpawnRelativeLocation);
		UE_VLOG_LOCATION(this, LogARPGAI, Verbose, SpawnWorldLocation, 1, FColor::Cyan, TEXT("Spawning %s"), *SpawnDescriptor.NpcId.ToString());
		auto SpawnedNpc = NpcGameMode->SpawnNpc(SpawnDescriptor.NpcId, SpawnWorldLocation, SpawnNpcWithTags);

		AliveNpcs.Add(SpawnedNpc);
		auto NpcAliveCreature = Cast<INpcAliveCreature>(SpawnedNpc);
		if (ensure(NpcAliveCreature))
			NpcAliveCreature->OnDeathStarted.AddUObject(this, &UNpcSpawnerComponent::OnNpcDied);

		if (bRegisterAsDesignatedZoneForNpc)
		{
			auto NpcComponent = SpawnedNpc->FindComponentByClass<UNpcComponent>();
			NpcComponent->SetDesignatedZone(this);
		}
		
		++NpcSpawnedThisTick;
		++SpawnedCount;
	}
	
	PendingNpcSpawnIndex += SpawnedCount;
	if (PendingNpcSpawnIndex >= TotalSpawns)
		SetComponentTickEnabled(false);
}
