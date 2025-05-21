// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_HitReactBase.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_Clash.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_Clash : public UGameplayAbility_CombatBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Clash();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FContextMontages> HitReacts;
	
	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* HitReactMontageTask;

	UFUNCTION()
	void OnHitReactMontageCompleted();

	UFUNCTION()
	void OnHitReactMontageInterrupted();

	UFUNCTION()
	void OnHitReactMontageCancelled();
};
