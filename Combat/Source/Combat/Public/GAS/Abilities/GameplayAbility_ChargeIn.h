// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_CombatBase.h"
#include "UObject/Object.h"
#include "GameplayAbility_ChargeIn.generated.h"

struct FContextMontages;
class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_ChargeIn : public UGameplayAbility_CombatBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_ChargeIn();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* ChargeInMontageTask;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FContextMontages> ChargeInMontageOptions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> ChargeInEffectClass;

private:
	FActiveGameplayEffectHandle ActiveChargeEffectHandle;

	UFUNCTION()
	void OnChargeInMontageCompleted();
	
	UFUNCTION()
	void OnChargeInMontageInterrupted();
	
	UFUNCTION()
	void OnChargeInMontageCancelled();
};
