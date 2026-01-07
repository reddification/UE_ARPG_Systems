// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeCombatComponent.h"
#include "Data/MeleeCombatSettings.h"
#include "PlayerSwingControlCombatComponent.generated.h"

class IPlayerCombatant;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UPlayerSwingControlCombatComponent : public UMeleeCombatComponent
{
	GENERATED_BODY()

private:
	struct FAccumulatedAttackVector
	{
		FVector2d RawInput = FVector2d::ZeroVector;
		FVector2d NormalizedDirection = FVector2d::ZeroVector;
		float RawInputSize = 0.f;
		float AngleRadians = 0.f; // TODO radians are not utilized much so perhaps remove them and refactor remaining code pieces to use degrees instead
		float AngleDegrees = 0.f;
		// float DotProduct = 0.f;
		EMeleeAttackType AttackType = EMeleeAttackType::None;
	};

	struct FAttackDirectionToAttackTypeMapping
	{
		float Radians = 0.f;
		EMeleeAttackType Attack = EMeleeAttackType::LeftMittelhauw;
	};
	
	DECLARE_MULTICAST_DELEGATE_TwoParams(FAttackInputProgressUpdatedEvent, EMeleeAttackType AttackType, float NewProgress);
	DECLARE_MULTICAST_DELEGATE_OneParam(FAttackStepDirectionUpdatedEvent, EAttackStepDirection StepDirection);
	DECLARE_MULTICAST_DELEGATE_OneParam(FRegisteringAttackStateChangedEvent, bool bEnabled);
		
public:
	UPlayerSwingControlCombatComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual bool RequestAttack(EMeleeAttackType RequestedAttackType) override;
	void RequestReleaseAttack();
	virtual void RequestReactivateAttack() override;

	virtual void StartComboWindow(const uint32 AttackAnimationId) override;
	virtual void EndComboWindow(const uint32 AttackAnimationId) override;

	virtual void BeginWindUp(float TotalDuration, const uint32 AttackAnimationId, EMeleeAttackType WindupAttackType) override;
	virtual void BeginRelease(float TotalDuration, const uint32 AttackAnimationId) override;
	virtual void BeginRecover(float TotalDuration, const uint32 AttackAnimationId) override;
	virtual void ResetAttackState() override;
	virtual void EndRecover(const uint32 AnimationId) override;
	
	virtual bool Feint() override;
	
	mutable FAttackInputProgressUpdatedEvent AttackInputProgressUpdatedEvent;
	mutable FAttackStepDirectionUpdatedEvent AttackStepDirectionUpdatedEvent;
	mutable FRegisteringAttackStateChangedEvent RegisteringAttackStateChangedEvent;
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TScriptInterface<IPlayerCombatant> OwnerPlayerCombat;
	
	TArray<FVector2d> CurrentPlayerInputs;
	double LastDistSqBetweenFocusedEnemyAndOwner = FLT_MAX;
	double DistSqToUpdateFocusedEnemy = 50.f * 50.f;

	void OnAttackStarted();
	void OnAttackReleased();

	// Not used for now
	void UpdateFocus(float DeltaTime);
	
	void IncreaseNoInputFrames();
	void ResetAttackAccumulationData();
	void RegisterNewInput(const FVector2d& Input);
	EMeleeAttackType GetAttackType(float Radians) const;
	
	EMeleeAttackType GetBasicAccumulatedAttack(TArray<FAccumulatedAttackVector>& AccumulatedVectors) const;
	EAttackStepDirection GetAttackStepDirection(const FVector& NormalizedAcceleration, const FVector& ForwardVector) const;
	int GetAttackActivationInputsCount() const;
	void DeduceComplexAttackInput(const TArray<FAccumulatedAttackVector>& AccumulatedVectors, EMeleeAttackType& FinalAttackType) const;
	
	void SetRegisteringAttack(bool bEnabled);
	void Attack();
	void MergeAttacks(TArray<FAccumulatedAttackVector>& AccumulatedVectors) const;

	bool bPlayerRequestsAttack = false;
	bool bRegisteringAttack = false;
	
	int NoInputFrames = 0;
	int CurrentAttackFrames = 0;
	int CurrentRegisteredInputIndex = 0;
	float CurrentAccelerationSignificance = 0.f;
	FMeleeCombatDirectionalInputParameters MeleeCombatParameters;
	FVector CurrentAcceleration = FVector::ZeroVector; // denotes direction where player wants to go. I guess it should be like decaying XY movement input
	
	TArray<FAttackDirectionToAttackTypeMapping> RadiansToAttackMapping;
	
	TWeakObjectPtr<AActor> FocusTarget = nullptr;
};