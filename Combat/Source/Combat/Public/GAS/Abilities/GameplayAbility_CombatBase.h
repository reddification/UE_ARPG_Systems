// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_BaseCombat.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_CombatBase.generated.h"

class UAbilityTask_WaitGameplayEvent;
/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_CombatBase : public UGameplayAbility_BaseCombat // TODO refactor, having _CombatBase and _BaseCombat is bullshit
{
	GENERATED_BODY()

public:
	UGameplayAbility_CombatBase();
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitAIAbortGameplayEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DirectionDotProductThreshold = 0.75f;
	
	FGameplayTag AbortTag;

private:
	UFUNCTION()
	void OnAIAbort(FGameplayEventData Payload);

	bool bWasCancelledByAI = false;
	bool bAIOwner = false;
};