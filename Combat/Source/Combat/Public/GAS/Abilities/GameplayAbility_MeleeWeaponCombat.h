// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_BaseCombat.h"
#include "Data/CombatDataTypes.h"
#include "Data/MeleeCombatSettings.h"
#include "GAS/Data/GameplayAbilityTargetData_Clash.h"
#include "Interfaces/ICombatant.h"
#include "GameplayAbility_MeleeWeaponCombat.generated.h"

class UMeleeCombatComponent;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class COMBAT_API UGameplayAbility_MeleeWeaponCombat : public UGameplayAbility_BaseCombat
{
	GENERATED_BODY()

public:
	UGameplayAbility_MeleeWeaponCombat();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	                        bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	virtual bool CanBeCanceled() const override;
	
protected:
	mutable TWeakObjectPtr<UMeleeCombatComponent> CombatComponentCached;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> HitRewardEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> HitWhiffedEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> CharacterStateDuringAttackEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectForOwnerWhenItsAttackBlockedClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DirectionDotProductThreshold = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool Debug_IgnoreAttackCost = false;

	UMeleeCombatComponent* GetMeleeCombatComponent();
	void InitializeCombatComponent();

	void OnAttackCommited();
	void OnAttackWhiffed();
	void OnAttackEnded();
	void OnAttackFeinted();
	
	virtual void OnAttackActivePhaseChanged(EMeleeAttackPhase NewAttackPhase, EMeleeAttackPhase OldAttackPhase);

	void ResetEventTask(UAbilityTask_WaitGameplayEvent*& AbilityTask);
	void TriggerClashAbility(AActor* OwnerActor, EClashSource ClashSource, const FVector& SweepDirection);

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	FGameplayTag GetHitDirectionTag(const AActor* HitActor, const FVector& HitDirection, const FVector& HitLocation) const;

	UFUNCTION()
	virtual void OnAbilityAborted(FGameplayEventData Payload);
	
private:
	void OnWeaponHit(UPrimitiveComponent* OtherActorComponent, const FHitResult& HitResult, EWeaponHitSituation WeaponHitSituation, const
	                 FVector& SweepDirection);
	void HandleEnemyHit(const UMeleeCombatSettings* CombatSettings, AActor* OwnerActor, AActor* EnemyActor,
	                    ICombatant* CombatantOwner, ICombatant* CombatantEnemy, const FHitResult& HitResult, const FVector& SweepDirection);
	void HandleWeaponsCollide(const UMeleeCombatSettings* CombatSettings, AActor* OwnerActor, AActor* EnemyActor,
	                          ICombatant* CombatantOwner, ICombatant* CombatantEnemy, const FVector& SweepDirection);
	void HandleAttackBlocked(AActor* EnemyActor, const FVector& Vector, const
	                         FHitResult& HitResult);
	void HandleAttackParried(AActor* Actor);
	
	void ApplyEffect(const TSubclassOf<UGameplayEffect>& EffectClass, float EffectLevel);
	
	TSubclassOf<UGameplayEffect> GetDamageGameplayEffect() const;
	FActiveGameplayEffectHandle ActiveAttackEffectHandle;

	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitForAbortEvent = nullptr;
};
