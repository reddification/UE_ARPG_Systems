// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/MeleeCombatSettings.h"
#include "MeleeCombatComponent.generated.h"

struct FGameplayTag;
class ICombatAnimInstance;
class ICombatant;

UCLASS()
class COMBAT_API UMeleeCombatComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE(FOnAttackEndedEvent);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttackActivePhaseChangedEvent, EMeleeAttackPhase OldAttackPhase, EMeleeAttackPhase NewAttackPhase);
	DECLARE_MULTICAST_DELEGATE_FourParams(FOnWeaponHitEvent, UPrimitiveComponent* OverlappedComponent, const FHitResult& SweepResult, EWeaponHitSituation HitSituation, const FVector& SweepDirection);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackStartedEvent, EMeleeAttackType AttackType)
	DECLARE_MULTICAST_DELEGATE(FOnAttackFeintedEvent)
	DECLARE_MULTICAST_DELEGATE(FOnAttackWhiffedEvent);
	DECLARE_MULTICAST_DELEGATE(FOnAttackCommitedEvent);
	DECLARE_MULTICAST_DELEGATE_OneParam(FAttackBlockWindowActiveChanged, bool bActive)
	
public:
	UMeleeCombatComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual bool RequestAttack(EMeleeAttackType RequestedAttackType);
	virtual void CancelAttack();
	virtual void ResetAttackState();
	virtual bool Feint();

	// NOTE!
	// these attach phases callback are called from AnimNotifyState class. These notifies are hand-places in all attacks animation assets
	// however, call order is not guaranteed! So BeginRelease can happen before EndWindUp. Keep it in mind and don't build logic where phases are dependent on each other
	virtual void BeginWindUp(float TotalDuration, const uint32 AnimationId, EMeleeAttackType WindupAttackTrajectoryType);
	virtual void BeginRelease(float TotalDuration, const uint32 AnimationId);
	virtual void BeginRecover(float TotalDuration, const uint32 AnimationId);
	virtual void EndWindUp(const uint32 AnimationId);
	virtual void EndRelease(const uint32 AnimationId);
	virtual void EndRecover(const uint32 AnimationId);

	void UpdateDamageCollisions();
	virtual void RequestReactivateAttack() {}
	virtual void StartComboWindow(const uint32 AttackAnimationId);
	virtual void EndComboWindow(const uint32 AttackAnimationId);

	FORCEINLINE EMeleeAttackPhase GetActiveAttackPhase() const { return AttackPhase; }
	FORCEINLINE double GetActiveAttackPhaseStartTime() const { return AttackPhase != EMeleeAttackPhase::None ? PhaseStartedAt : 0.f; }
	FORCEINLINE double GetActiveAttackPhaseEndTime() const { return AttackPhase != EMeleeAttackPhase::None ? PhaseEndsAt : 0.f; }
	FORCEINLINE bool IsAttacking() const { return AttackPhase == EMeleeAttackPhase::WindUp || AttackPhase == EMeleeAttackPhase::Release; }
	FORCEINLINE EMeleeAttackType GetActiveAttackType() const { return ActiveAttack; }
	FORCEINLINE EMeleeAttackType GetActiveAttackTrajectory() const { return ActiveAttackTrajectory; }
	FORCEINLINE int GetEnemiesHitThisAttack() const { return AliveActorsHit; }
	FORCEINLINE int GetCurrentComboTotalAttackCount() const { return CurrentComboTotalAttacksCount; }
	FORCEINLINE int GetCurrentComboHitAttackCount() const { return CurrentComboAttacksHitCount; }

	float GetAttackDamage(const FAttackDamageEvaluationData& AttackDamageEvaluationData, const FAttackDamageEvaluationData& EnemyDamageEvaluationData,
		const FGameplayTag& WeaponTypeTag, const FHitResult& HitResult);
	float GetPoiseDamage(const FAttackDamageEvaluationData& AttackDamageEvaluationData, const FAttackDamageEvaluationData& EnemyDamageEvaluationData,
		const FHitResult& HitResult);

	FVector GetActiveAttackDirection() const;
	bool CanCancel() const { return AttackPhase == EMeleeAttackPhase::None || AttackPhase == EMeleeAttackPhase::WindUp; }
	
	void OnAttackBlocked();
	bool CanChangeAttackToBlock();

	mutable FOnAttackStartedEvent OnAttackStartedEvent;
	mutable FOnAttackEndedEvent OnAttackEndedEvent;
	mutable FOnAttackCommitedEvent OnAttackCommitedEvent;
	mutable FOnWeaponHitEvent OnWeaponHitEvent;
	mutable FOnAttackWhiffedEvent OnAttackWhiffedEvent;
	mutable FOnAttackActivePhaseChangedEvent OnAttackActivePhaseChanged;
	
	mutable FOnAttackFeintedEvent OnAttackFeintedEvent;
	mutable FAttackBlockWindowActiveChanged AttackBlockWindowActiveChangedEvent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void FinalizeAttack();
	void SetAttackPhase(EMeleeAttackPhase AttackPhase, float TotalDuration);
	
	void SetDamageCollisionsEnabled(bool bEnabled);

	// TODO (29.01.2026) get it from combatant, and combatant should have it as a parameter somewhere (or even calculated based on character, aggression, stamina, etc)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ChanceToStopAttackingOnAttackBlocked = 0.75f;
	
	UPROPERTY()
	TArray<USkeletalMeshComponent*> WeaponCollisionsComponents;

	TArray<FCombatCollisionShapeData> WeaponCollisionShapes;
	
	TArray<FTransform> PreviousWeaponCollisionTransform;
	
	UPROPERTY()
	TScriptInterface<ICombatant> OwnerCombatant;

	UPROPERTY()
	TScriptInterface<ICombatAnimInstance> CombatAnimInstance;
	
	EMeleeAttackPhase AttackPhase;
	double PhaseEndsAt = 0.f;
	double PhaseStartedAt = 0.f;
	EMeleeAttackType ActiveAttack = EMeleeAttackType::None;
	EMeleeAttackType ActiveAttackTrajectory = EMeleeAttackType::None;

	FTimerHandle WeaponCollisionSweepsTimer;

	// can input next attack
	bool bComboWindowActive = false;
	// block ability can interrupt attack ability
	bool bBlockWindowActive = false;
	int CurrentComboTotalAttacksCount = 0;
	int CurrentComboAttacksHitCount = 0;

	float WeaponCollisionSweepsPerSecond = 60.f;
	FName WeaponCollisionProfileName = FName("Weapon");
	FName CombatCollisionCenterSocketName = FName("CollisionCenterSocket");
	FName CombatCollisionName = FName("CombatCollision");

	UPROPERTY()
	TSet<AActor*> CurrentAttackActorsHit;

	int AliveActorsHit = 0;
	
	UFUNCTION()
	virtual void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool BFromSweep, const FHitResult& SweepResult);

	void OnWeaponOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult, const FVector& SweepDirection);

	void CacheCollisionShapes();
	void SweepWeaponCollisions();

	virtual const TMap<int, FMeleeAttackPhaseSpeedModifier>& GetAttackPhasePlayRates() const;
	float GetAttackPhasePlayRate(EMeleeAttackPhase NewAttackPhase) const;
	void ReportAttackWhiffed();

	float ConsequitiveComboAttackWindUpSpeedScale = 1.5f;
	float KeepWeaponReadyAfterAttackDelay = 3.f;

	virtual void ActivateBlockWindow();
	virtual void StopBlockWindow();
	
protected:
	uint32 ActiveAnimationId = 0;
	uint32 ActiveComboWindowId = 0;
	float HeavyAttackWindupSpeedModifier = 0.7f;
	TWeakObjectPtr<const UMeleeCombatSettings> CachedMeleeCombatSettings;
	
private:
	FTimerHandle ActivateBlockWindowDelayTimer;
	FTimerHandle DeactivateBlockWindowTimer;
	
	void TryRequestBlockWindow(float TotalDuration);
	bool bNeedsReset = false;
};
