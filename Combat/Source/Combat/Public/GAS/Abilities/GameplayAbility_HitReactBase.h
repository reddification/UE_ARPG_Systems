// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_CombatBase.h"
#include "Data/CombatDataTypes.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_HitReactBase.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_HitReactBase : public UGameplayAbility_CombatBase
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FContextMontages> HitReacts;
	
	// routed to ICombatant. Can be used to distinguish between hit reacts, staggers, guard breaks, etc
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag HitTypeTag;

	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* HitReactMontageTask;

	UFUNCTION()
	void OnHitReactMontageCompleted();

	UFUNCTION()
	void OnHitReactMontageInterrupted();

	UFUNCTION()
	void OnHitReactMontageCancelled();
	
	FGameplayTagContainer ActiveHitReactTags;
};
