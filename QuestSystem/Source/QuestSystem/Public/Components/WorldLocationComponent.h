// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "WorldLocationComponent.generated.h"


struct FWorldLocationDTR;
class UBoxComponent;
class UQuestGiverComponent;
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QUESTSYSTEM_API UWorldLocationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetLocationIdTag() const { return LocationIdTag; }
	
	FVector GetRandomLocationInVolume(float FloorOffset = 100.f) const;
	FVector GetWorldLocation() const { return GetOwner()->GetActorLocation(); }
	const FWorldLocationDTR* GetLocationDTR() const;
	
protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, meta=(RowType="WorldLocationDTR"))
	FDataTableRowHandle WorldLocationDTRH;

	TWeakObjectPtr<UBoxComponent> CollisionComponent;

	FGameplayTag LocationIdTag;
	
private:
	TWeakObjectPtr<UQuestGiverComponent> QuestGiverComponent;
	bool bQuestLocation = false;

	UFUNCTION()
	void OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int OtherBodyIndex);

	void OnOverlappedActor(AActor* EnteredActor);

	void CheckOverlaps();
	TArray<AActor*> GetOverlappedActorsInVolume(const TSubclassOf<AActor>& ActorTypeOfInterest) const;
};
