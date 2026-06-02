// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "Npc.generated.h"

class UBlackboardComponent;

class INpcThreat;
// This class does not need to be modified.
UINTERFACE()
class UNpc : public UInterface
{
	GENERATED_BODY()
};

class ARPGAI_API INpc
{
	GENERATED_BODY()

public:
	virtual const FGameplayTag& GetNpcIdTag() const = 0;
	virtual const FText& GetNpcName() const = 0;
	virtual const FDataTableRowHandle& GetNpcDataTableRowHandle() const = 0;
	virtual UBlackboardComponent* GetBlackboard() const = 0;
	
	virtual float GetMoveSpeed() const = 0;
	virtual float GetCurrentSpeed() const = 0;
	virtual void SetForcedMoveSpeed(const float NewForcedMoveSpeed) = 0;
	virtual void ResetForcedMoveSpeed() = 0;
	
	// TODO refactor. Kinda bullshit decision. I need the AGameCharacter (project character) to store granted abilities handles
	// But from a solid standalone plugin architecture this is kinda bullshit - delegate such a thing to the owner
	virtual void GrantAbilitySet(const class UAbilitySet* AbilitySet) = 0;
	
	virtual void InterpolateToLocation_NPC(const FVector& Location, float InterpolateToSlotLocationRate, const TArray<const AActor*>& IgnoredActors, bool bSweep) = 0;

	virtual void LookAt(const FVector& Location) = 0;
	virtual void LookAt(const FRotator& Rotation) = 0;
	virtual void CancelLookAt() = 0;

	virtual bool IsAtLocation(const FGameplayTag& LocationId) const = 0;
	
	virtual bool TriggerArbitraryNpcEvent(const FGameplayTag& EventTag) = 0;
	
	virtual const AActor* GetCatchUpTarget() = 0;
	
	virtual bool PickUpItem_NPC(AActor* Item) = 0;
};
