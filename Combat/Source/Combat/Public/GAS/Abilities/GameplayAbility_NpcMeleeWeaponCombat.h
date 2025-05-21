// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_MeleeWeaponCombat.h"
#include "GameplayAbility_NpcMeleeWeaponCombat.generated.h"

UCLASS()
class COMBAT_API UGameplayAbility_NpcMeleeWeaponCombat : public UGameplayAbility_MeleeWeaponCombat
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	virtual void OnAttackActivePhaseChanged(EMeleeAttackPhase OldAttackPhase, EMeleeAttackPhase NewAttackPhase) override;
	
	virtual void OnAbilityAborted(FGameplayEventData Payload) override;
	
private:
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitForContinueAttackRequestEvent;
	
	UFUNCTION()
	void RequestNextAttack(FGameplayEventData Payload);
};