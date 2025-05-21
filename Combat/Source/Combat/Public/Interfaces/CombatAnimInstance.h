// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CombatDataTypes.h"
#include "UObject/Interface.h"
#include "CombatAnimInstance.generated.h"

// This class does not need to be modified.
UINTERFACE()
class COMBAT_API UCombatAnimInstance : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class COMBAT_API ICombatAnimInstance
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	FSimpleDelegate OnLastComboAttackEvent;
	
	virtual void SetAttackPhase(EMeleeAttackPhase AttackPhase) = 0;
	virtual void SetAttackPlayRate(const float InPlayRate) = 0;
	virtual void SetAttack(EMeleeAttackType MeleeAttack, const FVector& InRelativeAttackAcceleration, EAttackStepDirection InAttackStepDirection, int ComboAttackCount) = 0;
	virtual void SetAttack(EMeleeAttackType MeleeAttack, const FVector& InRelativeAttackAcceleration, EAttackStepDirection InAttackStepDirection, int ComboAttackCount, int RandomAttackAnimationIndex) = 0;
	virtual void OnAttackWindUpBegin() = 0;
	virtual void SetParriedAttack(EMeleeAttackType ParriedAttack) = 0;
	virtual void SetBlocking(bool bInBlocking) = 0;
	virtual void SetBlockPosition(const FVector2D& NewBlockPosition) = 0;
	virtual void OnAttackFinished() = 0;
	virtual void SetReadyUpWeapon(bool bEnemyClose) = 0;
	virtual void KeepWeaponReady(float KeepWeaponReadyAfterAttackDelay) = 0;
};
