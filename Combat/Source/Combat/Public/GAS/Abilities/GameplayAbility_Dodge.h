// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_CombatBase.h"
#include "Abilities/GameplayAbility.h"
#include "UObject/Object.h"
#include "GameplayAbility_Dodge.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_Dodge : public UGameplayAbility_CombatBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Dodge();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* DodgeMontageTask;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<struct FContextMontages> DodgeMontageOptions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DodgeEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve AnimationSpeedDexterityDependence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FRuntimeFloatCurve RootMotionTranslationScaleDexterityDependence;

private:
	FActiveGameplayEffectHandle ActiveDodgeEffectHandle;

	UFUNCTION()
	void OnDodgeMontageCompleted();
	
	UFUNCTION()
	void OnDodgeMontageInterrupted();
	
	UFUNCTION()
	void OnDodgeMontageCancelled();
};
