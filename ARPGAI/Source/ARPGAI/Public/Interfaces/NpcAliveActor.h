// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "NpcAliveActor.generated.h"

struct FGameplayAttribute;

// This class does not need to be modified.
UINTERFACE()
class ARPGAI_API UNpcAliveActor : public UInterface
{
	GENERATED_BODY()
};

// it's empty now but in the future likely to be expanded with death cause data
struct FNpcDeathEventData
{
	FNpcDeathEventData(AActor* InCauser) { Causer = InCauser; }
	FNpcDeathEventData(AActor* InCauser, const FGameplayTag& InLastHitType) 
	{ Causer = InCauser; LastHitType = InLastHitType; }
	
	TWeakObjectPtr<AActor> Causer = nullptr;
	FGameplayTag LastHitType;
};

class ARPGAI_API INpcAliveActor
{
	GENERATED_BODY()
	
private:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FNpcDeathStartedEvent, AActor* OwningActor, const FNpcDeathEventData& DeathEventData);
	DECLARE_MULTICAST_DELEGATE_OneParam(FNpcDeathFinishedEvent, AActor* OwningActor);
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetHealth_NPC() const = 0;
	virtual float GetMaxHealth_NPC() const = 0;

	virtual float GetStamina_NPC() const = 0;
	virtual float GetMaxStamina_NPC() const = 0;
	
	virtual bool IsAlive_NPC() const = 0;
	
	// 31 Mar 2026 (aki): TODO decouple all GAS related stuff into a separate ARPG_AIxGAS module
	virtual FGameplayAttribute GetHealthAttribute_NPC() const = 0;
	virtual FGameplayAttribute GetMaxHealthAttribute_NPC() const = 0;
	virtual FGameplayAttribute GetPoiseAttribute_NPC() const = 0;
	virtual FGameplayAttribute GetMaxPoiseAttribute_NPC() const = 0;
	virtual FGameplayAttribute GetStaminaAttribute_NPC() const = 0;
	virtual FGameplayAttribute GetMaxStaminaAttribute_NPC() const = 0;

	virtual FGameplayTag GetTagId_NPC() const = 0;
	virtual FGuid GetId_NPC() const = 0;
	
	mutable FNpcDeathStartedEvent OnNpcAliveActorDeathStarted;
	mutable FNpcDeathFinishedEvent OnNpcAliveActorDeathFinished;
};
