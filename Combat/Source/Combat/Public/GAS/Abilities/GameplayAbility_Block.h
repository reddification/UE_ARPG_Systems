// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_BaseCombat.h"
#include "GameplayAbility_CombatBase.h"
#include "Abilities/GameplayAbility.h"
#include "Data/CombatDataTypes.h"
#include "GameplayAbility_Block.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_Block : public UGameplayAbility_BaseCombat
{
	GENERATED_BODY()

public:
	UGameplayAbility_Block();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	virtual bool StartBlocking(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData);
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnAttackParried(UActorComponent* ActorComponent, const FHitResult& HitResult, const FVector& Vector);
	virtual void OnAttackBlocked(float ConsumptionScale);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> ActiveBlockEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> AttackBlockedEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> AttackParriedEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FContextMontages> HitReacts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float BaseBlockStaminaConsumption = 15.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float BaseBlockPoiseConsumption = 3.f;
	
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitAbortGameplayEvent;

private:
	FActiveGameplayEffectHandle ActiveEffectSpecHandle;
	
	UFUNCTION()
	void OnAbort(FGameplayEventData Payload);
	
	void ApplyInstantEffect(const TSubclassOf<UGameplayEffect>& EffectClass);
};
