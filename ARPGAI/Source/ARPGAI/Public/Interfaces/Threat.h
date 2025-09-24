// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Threat.generated.h"

// This class does not need to be modified.
UINTERFACE()
class ARPGAI_API UThreat : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API IThreat
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE_OneParam(FThreatCombatCommonEvent, AActor* ThreatActor);
	
public:
	virtual float GetThreatLevel(const AActor* ThreatOwner) const = 0;
	virtual float GetAttackRange() const = 0;
	virtual float GetStrength() const = 0;
	virtual float GetAverageProtection() const = 0;
	virtual bool IsAttacking() const = 0;
	virtual float GetHealth() const = 0;
	virtual bool IsStaggered() const = 0;
	virtual FVector GetThreatLocation() const = 0;
	virtual void ReportPreparingAttack(APawn* Attacker, bool bActive) = 0;
	virtual FGameplayTagContainer GetAttitudeTags() const = 0;
	virtual bool CanSeeThreat(APawn* Target) const = 0;

	FThreatCombatCommonEvent OnEnemyPerformingAttackEvent;
	FThreatCombatCommonEvent OnEnemyFeintedAttackEvent;
	FThreatCombatCommonEvent OnEnemyAttackWhiffedEvent;
	FThreatCombatCommonEvent OnEnemyBlockEvent;
	FThreatCombatCommonEvent OnEnemyWeaponChangedEvent;
};
