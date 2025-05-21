// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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
	virtual float GetHealth() const = 0;
	virtual float GetMaxHealth() const = 0;

	virtual float GetStamina() const = 0;
	virtual float GetMaxStamina() const = 0;

	virtual bool IsNpcActorAlive() const = 0;
	
	virtual FGameplayAttribute GetHealthAttribute() const = 0;
	virtual FGameplayAttribute GetMaxHealthAttribute() const = 0;
	virtual FGameplayAttribute GetPoiseAttribute() const = 0;
	virtual FGameplayAttribute GetMaxPoiseAttribute() const = 0;
	virtual FGameplayAttribute GetStaminaAttribute() const = 0;
	virtual FGameplayAttribute GetMaxStaminaAttribute() const = 0;

	mutable FNpcDeathEvent OnDeathStarted;
	mutable FNpcDeathEvent OnDeathFinished;
};
