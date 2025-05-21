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
	
	CollisionComponent = IWorldLocationInterface::Execute_GetOverlapCollision(GetOwner());
	if (!ensure(CollisionComponent.IsValid()))
		return;
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UWorldLocationComponent::CheckOverlaps);
}

void UWorldLocationComponent::BeginDestroy()
{
	auto World = GetWorld();
	if (World)
	{
		auto WLS = World->GetSubsystem<UWorldLocationsSubsystem>();
		if (WLS)
			WLS->UnregisterWorldLocation(LocationIdTag);
	}
	
	Super::BeginDestroy();
}

void UWorldLocationComponent::OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OnOverlappedActor(OtherActor);
}

void UWorldLocationComponent::OnExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int OtherBodyIndex)
{
	if (LocationIdTag.IsValid())
	{
		auto QuestCharacter = Cast<IQuestCharacter>(OtherActor);
		if (!QuestCharacter)
			return;
		
		QuestCharacter->RemoveQuestTags(LocationIdTag.GetSingleTagContainer());
		if (bQuestLocation)
		{
			auto QuestSystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuestSubsystem>();
			QuestSystem->OnLocationLeft(LocationIdTag, QuestCharacter);    
		}
	}
}

void UWorldLocationComponent::OnOverlappedActor(AActor* EnteredActor)
{
	auto QuestCharacter = Cast<IQuestCharacter>(EnteredActor);
	if (!ensure(QuestCharacter))
		return;
	
	if (bQuestLocation)
	{
		auto QuestSystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuestSubsystem>();
		QuestSystem->OnLocationReached(LocationIdTag, QuestCharacter);    
	}

	if (QuestGiverComponent.IsValid() && QuestCharacter->IsPlayer())
		QuestGiverComponent->GiveQuests();

	if (LocationIdTag.IsValid())
		QuestCharacter->AddQuestTags(LocationIdTag.GetSingleTagContainer());
}

void UWorldLocationComponent::CheckOverlaps()
{
	if (!CollisionComponent->OnComponentBeginOverlap.IsBound())
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &UWorldLocationComponent::OnOverlapped);

	if (!CollisionComponent->OnComponentEndOverlap.IsBound())
		CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &UWorldLocationComponent::OnExit);
	
	TArray<AActor*> InitiallyOverlappyingActors = GetOverlappedActorsInVolume(APawn::StaticClass());
	if (InitiallyOverlappyingActors.Num() > 0)
	{
		for (auto Actor : InitiallyOverlappyingActors)
			OnOverlappedActor(Actor);
	}
}

TArray<AActor*> UWorldLocationComponent::GetOverlappedActorsInVolume(const TSubclassOf<AActor>& ActorTypeOfInterest) const
{
	TArray<AActor*> Result;
	CollisionComponent->GetOverlappingActors(Result, ActorTypeOfInterest);
	return Result;
}

FVector UWorldLocationComponent::GetRandomLocationInVolume(float FloorOffset) const
{
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
