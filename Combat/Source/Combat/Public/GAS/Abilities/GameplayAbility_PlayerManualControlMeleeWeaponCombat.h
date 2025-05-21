// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_MeleeWeaponCombat.h"
#include "UObject/Object.h"
#include "GameplayAbility_PlayerManualControlMeleeWeaponCombat.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_PlayerManualControlMeleeWeaponCombat : public UGameplayAbility_MeleeWeaponCombat
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitForAttackReleaseRequest;

	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitForReactivateAttackRequest;

	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitForFeintRequest;

	UFUNCTION()
	void OnAttackReleaseRequested(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackReactivateRequested(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackFeintRequested(FGameplayEventData Payload);
};
