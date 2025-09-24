// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_BaseCombat.h"
#include "Abilities/GameplayAbility.h"
#include "Data/CombatDataTypes.h"
#include "UObject/Object.h"
#include "GameplayAbility_Death.generated.h"

struct FContextMontages;

UCLASS()
class COMBAT_API UGameplayAbility_Death : public UGameplayAbility_BaseCombat
{
	GENERATED_BODY()

public:
	UGameplayAbility_Death();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// map death sources to montages
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FContextMontages> DeathMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMontageData> DefaultDeathMontages;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UAnimMontage>> DefaultDeathMontageOptions_Deprecated;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=DeathOverrides)
	TMap<TSoftObjectPtr<USkeletalMesh>, TObjectPtr<UAnimSequence>> DeathSkeletonOverride_Deprecated;

	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* MontageTask;

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageInterrupted();

	TSoftObjectPtr<USkeletalMesh> InitialSkeletalMesh;
};
