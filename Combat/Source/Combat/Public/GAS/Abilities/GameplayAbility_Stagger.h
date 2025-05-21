// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_HitReactBase.h"
#include "GameplayAbility_Stagger.generated.h"

struct FContextMontages;
class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_Stagger : public UGameplayAbility_HitReactBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Stagger();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> StaggerRecoverEffectClass;

private:
	FActiveGameplayEffectHandle ActiveStaggerEffectHandle;
};

