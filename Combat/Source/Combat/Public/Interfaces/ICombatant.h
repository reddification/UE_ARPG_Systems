// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Data/CombatDataTypes.h"
#include "ICombatant.generated.h"

class ICombatAnimInstance;
// This class does not need to be modified.
UINTERFACE()
class UCombatant : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class COMBAT_API ICombatant
{
	GENERATED_BODY()

public:
	FSimpleDelegate OnCombatantWeaponChanged;
	
	virtual void OnAttackRequested(EMeleeAttackType Attack, const FVector& Direction, const FVector& InRelativeAttackAcceleration) = 0;

	virtual void SetCombatantMovementEnabled(const FGameplayTag& LockTag, bool bEnabled) = 0;
	virtual void SetAttackPhase(EMeleeAttackPhase NewAttackPhase, EMeleeAttackPhase OldAttackPhase) = 0;

	virtual TScriptInterface<ICombatAnimInstance> GetCombatAnimInstance() const = 0;
	virtual float GetAttackRange() const = 0;
	virtual int GetActiveWeaponMasteryLevel() const = 0;
	virtual FGameplayTag GetActiveCombatStyleTag() const = 0;
	virtual FGameplayTag GetActiveWeaponTypeTag() const = 0;
	virtual EMeleeAttackType GetActiveAttack() const = 0;
	virtual EMeleeAttackType GetActiveAttackTrajectory() const = 0;
	virtual ECollisionChannel GetWeaponCollisionObjectType() const = 0;
	virtual ECollisionChannel GetBodyCollisionObjectType() const = 0;
	virtual TSubclassOf<UGameplayEffect> GetDamageEffect() const = 0;
	virtual float GetStrength() const = 0;
	virtual float GetDexterity() const = 0;
	virtual float GetStaminaRatio() const = 0;
	virtual float GetPoise() const = 0;
	virtual float GetPoiseDamageScale() const = 0;
	virtual FWeaponDamageData GetWeaponDamageData() const = 0;
	virtual void TakeHit(const FReceivedHitData& ReceivedHitData) = 0;

	virtual void OnStaggerStarted(const FGuid& StaggeredBy) = 0;
	virtual void OnStaggerFinished() = 0;
	virtual float GetStaggerPoiseThreshold() const = 0;
	virtual float GetStaggerDuration() const = 0;
	virtual void OnStaggeredActor(AActor* Actor) {}
	virtual const FGuid& GetCombatantId() const = 0;

	virtual void OnAttackStarted() = 0;
	virtual void OnAttackFeinted() = 0;
	virtual void OnAttackWhiffed() = 0;
	virtual void OnAttackEnded() = 0;
	virtual void OnAttackCanceled() = 0;
	virtual void OnBlockSet() = 0;
	
	virtual const FReceivedHitData& GetLastHitData() = 0;
	
	virtual void FinishDeath() = 0;
	virtual void StartDeath() = 0;

	virtual void FinishKnockdown() = 0;
	virtual void StartKnockdown() = 0;
	
	virtual void ChargeInStarted() = 0;
	virtual void ChargeInFinished() = 0;
	virtual void ChargeInCanceled() = 0;

	virtual void DodgeStarted(const FVector& DodgeDirectionWorld) = 0;
	virtual void DodgeFinished() = 0;
	virtual void DodgeCanceled() = 0;

	virtual TArray<USkeletalMeshComponent*> GetDamageCollisionsComponents() const = 0;

	virtual USkeletalMeshComponent* GetBlockCollisionsComponent() const = 0;
	virtual void SetBlocking(bool bEnabled) = 0;
	virtual bool IsBlocking() const = 0;
	virtual void OnBlockPeakReached(const FVector2D& BlockDirection) = 0;

	virtual const FWeaponFX* GetWeaponFX(const FGameplayTag& FXSourceTag) const = 0;
	virtual void PlayCombatSound(const FGameplayTag& SoundTag) const = 0;
	virtual TSet<AActor*> GetCombatObservedActors() const = 0;
	virtual void PlayCombatMontage(UAnimMontage* Montage) const = 0;
	virtual AActor* GetTarget() const = 0;

	virtual float GetCombatantCapsuleRadius() const = 0;
	virtual float GetCombatantCapsuleHalfHeight() const = 0;
	virtual float GetWeaponAttackPhaseSpeedScale(EMeleeAttackPhase NewAttackPhase) const = 0;

	virtual bool IsUsingShield() const = 0;
	virtual bool IsUsingRangeWeapon() const { return false; }
	virtual ECollisionComponentWeaponType GetCollisionWeaponType(UPrimitiveComponent* WeaponCollisionComponent) = 0;
	virtual USkeletalMeshComponent* GetCombatantSkeletalMeshComponent() const = 0;

	virtual void ActivateCombatantRagdoll() = 0;
	virtual void FlinchWeapon(const FVector& HitDirectionWorld) = 0;
};
