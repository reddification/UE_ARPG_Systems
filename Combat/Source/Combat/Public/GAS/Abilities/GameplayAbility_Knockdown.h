// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_BaseCombat.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameplayAbility_Knockdown.generated.h"

struct FContextMontages;

UCLASS()
class COMBAT_API UGameplayAbility_Knockdown : public UGameplayAbility_BaseCombat
{
	GENERATED_BODY()

public:
	UGameplayAbility_Knockdown();
	
	UFUNCTION()
	void TimerFinish();

	UFUNCTION()
	void OnWakeUpMontageCompleted();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float KnockdownRecoverTime = 3.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* KnockdownMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* WakeUpMontageMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> RecoverEffects;

	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* MontageTask;

private:
	UPROPERTY()
	UAbilityTask_WaitDelay* WaitTask;
};

