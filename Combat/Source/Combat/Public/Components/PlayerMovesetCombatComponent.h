// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeCombatComponent.h"
#include "Components/ActorComponent.h"
#include "Data/MeleeCombatSettings.h"
#include "PlayerMovesetCombatComponent.generated.h"

class IPlayerCombatant;
struct FWeaponMasteryMoveset;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UPlayerMovesetCombatComponent : public UMeleeCombatComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void FindAutoTarget(AActor* OwnerLocal, const FVector& OwnerLocation, const FVector& TargetDirection);

	virtual bool RequestAttack(EMeleeAttackType RequestedAttackType) override;

	// virtual void BeginWindUp(float TotalDuration, const uint32 AnimationId) override;
	virtual void ResetAttackState() override;

protected:
	virtual void BeginPlay() override;

	virtual void BeginWindUp(float TotalDuration, const uint32 AnimationId, EMeleeAttackType WindupAttackType) override;
	virtual void BeginRelease(float TotalDuration, const uint32 AnimationId) override;
	virtual void BeginRecover(float TotalDuration, const uint32 AnimationId) override;
	virtual void EndRecover(const uint32 AnimationId) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AutoTargetRadius = 300.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RotationRate = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int MisinputsOutsideOfComboWindowToResetAttack = 3;
	
	bool bLastComboAttack = false;

private:
	void OnLastComboAttack();
	
	int MovesetAttackIndex = 0;
	int CurrentMisinputs = 0;
	FRotator LastValidPlayerInputMovementRequestRotator = FRotator::ZeroRotator;
	FVector InitialCombatantViewDirection = FVector::ZeroVector;
	
	UPROPERTY()
	TScriptInterface<IPlayerCombatant> OwnerPlayerCombat;
	
	TWeakObjectPtr<AActor> ManualTarget;
	TWeakObjectPtr<AActor> AutoTarget;
};
