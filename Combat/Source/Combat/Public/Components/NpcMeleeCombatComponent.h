// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MeleeCombatComponent.h"
#include "NpcMeleeCombatComponent.generated.h"


class INpcCombatant;
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UNpcMeleeCombatComponent : public UMeleeCombatComponent
{
	GENERATED_BODY()

public:
	virtual bool RequestAttack(EMeleeAttackType RequestedAttackType) override;

	bool RequestNextAttack();
	virtual void ResetAttackState() override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EndRecover(const uint32 AnimationId) override;
	
	virtual const TMap<int, FMeleeAttackPhaseSpeedModifier>& GetAttackPhasePlayRates() const override;
	
private:
	TWeakObjectPtr<AAIController> AIController;
	TScriptInterface<INpcCombatant> OwnerNPCCombatant;
	int WeaponMasteryLevel = 1;
	FTimerHandle ResetPreviousAttackTimer;
	int RequestedAttacksCount = 0;
	
	void ResetPreviousAttack();
	float AddIntelligenceMisperception(float BaseValue, float Intelligence, const FRuntimeFloatCurve& RuntimeFloatCurve);
};
