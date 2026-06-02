// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "NpcThreat.generated.h"

// This class does not need to be modified.
UINTERFACE()
class ARPGAI_API UNpcThreat : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcThreat
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE_OneParam(FThreatCombatCommonEvent, AActor* ThreatActor);
	
public:
	virtual float GetThreatLevel(const AActor* ThreatOwner) const = 0;
	virtual float GetAttackRange_NpcThreat() const = 0;
	virtual float GetDamageOutput_NpcThreat() const = 0;
	virtual float GetAverageProtection_NpcThreat() const = 0;
	virtual bool IsAttacking_NpcThreat() const = 0;
	virtual float GetHealth_NpcThreat() const = 0;
	virtual bool IsStaggered_NpcThreat() const = 0;
	virtual FVector GetThreatLocation() const = 0;
	virtual void ReportPreparingAttack(APawn* Attacker, bool bActive) = 0;
	virtual FGameplayTagContainer GetAttitudeTags() const = 0;
	virtual bool CanSeeThreat(APawn* Target) const = 0;

	FThreatCombatCommonEvent OnThreatStartedAttackEvent;
	FThreatCombatCommonEvent OnThreatAttackCompletedEvent;
	FThreatCombatCommonEvent OnThreatFeintedAttackEvent;
	FThreatCombatCommonEvent OnThreatAttackWhiffedEvent;
	FThreatCombatCommonEvent OnThreatBlockEvent;
	FThreatCombatCommonEvent OnThreatWeaponChangedEvent;
};
