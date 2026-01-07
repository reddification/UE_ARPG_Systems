// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "PlayerCombat.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPlayerCombatant : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class COMBAT_API IPlayerCombatant
{
	GENERATED_BODY()

public:
	virtual FVector2D ConsumeCurrentAttackInput() const = 0;
	virtual FVector2D ConsumeCurrentBlockInput() const = 0;
	
	virtual void SetLookEnabled(bool bEnabled) = 0;
	virtual ECollisionChannel GetTargetCollisionChannel() const = 0;
	virtual FVector GetCombatMovementDirection() const = 0;
	virtual FVector ConsumeCombatMovementRawInput() const = 0;
	virtual void EnableAttackCameraDampering(float MaxCameraRotationAngleDuringAttack, float LookDamperingRatio) = 0;
	virtual void DisableAttackCameraDampering() = 0;
	virtual void SetCombatFocus(AActor* FocusedActor) = 0;
	virtual void ResetCombatFocus() = 0;
	virtual FRotator GetPlayerCombatantViewDirection() const = 0;
	virtual void SetOrientationFollowsAttack(bool bActive) = 0;
	virtual void PlayCameraShake_Combat(const FGameplayTag& CameraShakeTag) = 0;
};
