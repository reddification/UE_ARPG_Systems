// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WorldLocationComponent.h"

#include "Components/ShapeComponent.h"
#include "Data/WorldLocationDTR.h"
#include "Interfaces/QuestCharacter.h"
#include "Interfaces/WorldLocationInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Subsystems/QuestSubsystem.h"
#include "Subsystems/WorldLocationsSubsystem.h"
#include "NavigationSystem.h"
#include "Components/BoxComponent.h"

const FWorldLocationDTR* UWorldLocationComponent::GetLocationDTR() const
{
	return WorldLocationDTRH.DataTable != nullptr && WorldLocationDTRH.RowName.IsValid() ? WorldLocationDTRH.GetRow<FWorldLocationDTR>("") : nullptr;
}

// Called when the game starts
void UWorldLocationComponent::BeginPlay()
{
	Super::BeginPlay();
	auto WorldLocationInterface = Cast<IWorldLocationInterface>(GetOwner());
	if (!ensure(WorldLocationInterface))
		return;

	QuestGiverComponent = WorldLocationInterface->GetQuestGiverComponent();
	
	if (ensure(WorldLocationDTRH.DataTable && !WorldLocationDTRH.RowName.IsNone()))
	{
		if (auto WorldLocationDTR = WorldLocationDTRH.GetRow<FWorldLocationDTR>(""))
			bQuestLocation = WorldLocationDTR->bQuestLocation;
	}

	LocationIdTag = FGameplayTag::RequestGameplayTag(WorldLocationDTRH.RowName, false);
	if (ensure(LocationIdTag.IsValid()))
	{
		auto WLS = GetWorld()->GetSubsystem<UWorldLocationsSubsystem>();
		WLS->RegisterWorldLocation(this, LocationIdTag);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("Location tag %s not found in game tags"), *WorldLocationDTRH.RowName.ToString()));
	}

	auto CollisionComponentsRaw = WorldLocationInterface->GetWorldLocationVolumes();
	CollisionComponents.Reserve(CollisionComponentsRaw.Num());
	for (auto* CollisionComponent : CollisionComponentsRaw)
		CollisionComponents.Emplace(CollisionComponent);
	
	if (!ensure(!CollisionComponents.IsEmpty()))
		return;
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UWorldLocationComponent::CheckOverlaps);
}

void UWorldLocationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	auto World = GetWorld();
	if (World)
	{
		auto WLS = World->GetSubsystem<UWorldLocationsSubsystem>();
		if (WLS)
			WLS->UnregisterWorldLocation(LocationIdTag);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UWorldLocationComponent::OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OnEnter(OtherActor);
}

void UWorldLocationComponent::OnExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int OtherBodyIndex)
{
	auto QuestCharacter = Cast<IQuestCharacter>(OtherActor);
	if (!QuestCharacter)
		return;
	
	if (LocationIdTag.IsValid() && bQuestLocation)
	{
		auto QuestSystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuestSubsystem>();
		QuestSystem->OnLocationLeft(LocationIdTag, QuestCharacter);    
	}

	if (LocationIdTag.IsValid())
		QuestCharacter->OnWorldLocationLeft(LocationIdTag);
	
	if (!LocationIndividualTags.IsEmpty())
		QuestCharacter->RemoveQuestTags(LocationIndividualTags);

	if (auto LocationDTR = GetLocationDTR())
	{
		if (LocationDTR->LocationTags.IsValid())
			QuestCharacter->RemoveQuestTags(LocationDTR->LocationTags);

		for (const auto& LocationCrossedHandler : LocationDTR->LocationCrossedHandlers)
			LocationCrossedHandler.Get<FWorldLocationCrossedHandler>().OnLocationCrossed(this, OtherActor, false);
	}
}

void UWorldLocationComponent::OnEnter(AActor* EnteredActor)
{
	auto QuestCharacter = Cast<IQuestCharacter>(EnteredActor);
	if (QuestCharacter == nullptr)
		return;

	if (LocationIdTag.IsValid() && bQuestLocation)
	{
		auto QuestSystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuestSubsystem>();
		QuestSystem->OnLocationReached(LocationIdTag, QuestCharacter);    
	}

	if (QuestGiverComponent.IsValid() && QuestCharacter->IsPlayer())
		QuestGiverComponent->GiveQuests();

	if (LocationIdTag.IsValid())
		QuestCharacter->OnWorldLocationEntered(LocationIdTag);
	
	if (!LocationIndividualTags.IsEmpty())
		QuestCharacter->AddQuestTags(LocationIndividualTags);

	auto LocationDTR = GetLocationDTR();
	if (LocationDTR != nullptr)
	{
		if (LocationDTR->LocationTags.IsValid())
			QuestCharacter->AddQuestTags(LocationDTR->LocationTags);
	
		for (const auto& LocationCrossedHandler : LocationDTR->LocationCrossedHandlers)
			LocationCrossedHandler.Get<FWorldLocationCrossedHandler>().OnLocationCrossed(this, EnteredActor, true);
	}
}

void UWorldLocationComponent::CheckOverlaps()
{
	for (auto& CollisionComponent : CollisionComponents)
	{
		if (!CollisionComponent->OnComponentBeginOverlap.IsBound())
			CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &UWorldLocationComponent::OnOverlapped);

		if (!CollisionComponent->OnComponentEndOverlap.IsBound())
			CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &UWorldLocationComponent::OnExit);
	}
	
	TArray<AActor*> InitiallyOverlappyingActors = GetOverlappedActorsInVolume(APawn::StaticClass());
	if (InitiallyOverlappyingActors.Num() > 0)
	{
		for (auto Actor : InitiallyOverlappyingActors)
			OnEnter(Actor);
	}
}

TArray<AActor*> UWorldLocationComponent::GetOverlappedActorsInVolume(const TSubclassOf<AActor>& ActorTypeOfInterest) const
{
	TArray<AActor*> Result;
	for (auto& CollisionComponent : CollisionComponents)
		CollisionComponent->GetOverlappingActors(Result, ActorTypeOfInterest);
	
	return Result;
}

// @AK 12.07.2025: doesn't really belong to quest system domain. perhaps move world location to a separate plugin or make a child component in game module 
bool UWorldLocationComponent::IsPointWithinArea(const FVector& TestLocation, const float AreaExtent) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UWorldLocationComponent::IsPointWithinArea)

	for (const auto& MarkupVolumeActorComponent : CollisionComponents)
	{
		const FVector LocalPoint = MarkupVolumeActorComponent->GetComponentTransform().InverseTransformPosition(TestLocation);
		const FVector BoxExtent = MarkupVolumeActorComponent->GetScaledBoxExtent() + FVector(AreaExtent);
		if (FMath::Abs(LocalPoint.X) <= BoxExtent.X &&
			FMath::Abs(LocalPoint.Y) <= BoxExtent.Y &&
			FMath::Abs(LocalPoint.Z) <= BoxExtent.Z)
		{
			return true;
		}
	}

	return false;
}

FVector UWorldLocationComponent::GetRandomLocationInVolume(float FloorOffset) const
{
	auto* CollisionComponent = CollisionComponents.Num() > 1
		? CollisionComponents[FMath::RandRange(0, CollisionComponents.Num() - 1)].Get()
		: CollisionComponents[0].Get();
	
	// TODO shape sweep if you can actually place an actor here
	FVector ComponentLocation = CollisionComponent->GetComponentLocation();
	FVector ScaledBoxExtent = CollisionComponent->GetScaledBoxExtent();
	FVector NavigableSpawnLocation;
	bool bRandomNavigableLocationFound = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), ComponentLocation,
		NavigableSpawnLocation, ScaledBoxExtent.Size2D());
	if (bRandomNavigableLocationFound)
		return NavigableSpawnLocation + FVector::UpVector * FloorOffset;
	else
		ensure(false);
	
	FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(ComponentLocation, ScaledBoxExtent * 0.9);
	RandomPoint.Z = ComponentLocation.Z - ScaledBoxExtent.Z + FloorOffset;
	return RandomPoint;
}

FVector UWorldLocationComponent::GetWorldLocation(const FVector& QuerierLocation) const
{
	if (auto WorldLocationInterface = Cast<IWorldLocationInterface>(GetOwner()))
		return WorldLocationInterface->GetWorldLocationNavigationLocation(QuerierLocation);

	return GetOwner()->GetActorLocation();
}
