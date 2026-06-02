// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "NpcAliveCreature.generated.h"

struct FGameplayAttribute;

// This class does not need to be modified.
UINTERFACE()
class ARPGAI_API UNpcAliveCreature : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcAliveCreature
{
	GENERATED_BODY()
	
private:
	DECLARE_MULTICAST_DELEGATE_OneParam(FNpcDeathEvent, AActor* OwningActor);
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetHealth_NpcAliveCreature() const = 0;
	virtual float GetMaxHealth_NpcAliveCreature() const = 0;

	virtual float GetNpcAliveCreatureStamina() const = 0;
	virtual float GetNpcAliveCreatureMaxStamina() const = 0;
	
	virtual bool IsAlive_NpcAliveCreature() const = 0;
	
	// 31 Mar 2026 (aki): TODO decouple all GAS related stuff into a separate ARPG_AIxGAS module
	virtual FGameplayAttribute GetHealthAttribute_NpcAliveCreature() const = 0;
	virtual FGameplayAttribute GetMaxHealthAttribute_NpcAliveCreature() const = 0;
	virtual FGameplayAttribute GetPoiseAttribute_NpcAliveCreature() const = 0;
	virtual FGameplayAttribute GetMaxPoiseAttribute_NpcAliveCreature() const = 0;
	virtual FGameplayAttribute GetStaminaAttribute_NpcAliveCreature() const = 0;
	virtual FGameplayAttribute GetMaxStaminaAttribute_NpcAliveCreature() const = 0;

	virtual FGameplayTag GetTagId_NpcAliveCreature() const = 0;
	virtual FGuid GetId_NpcAliveCreature() const = 0;
	
	mutable FNpcDeathEvent OnNpcAliveCreatureDeathStarted;
	mutable FNpcDeathEvent OnNpcAliveCreatureDeathFinished;
};
