// 

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "NpcCombatInterface.generated.h"

class INpcThreat;
// This class does not need to be modified.
UINTERFACE()
class UNpcCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcCombatInterface
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_TwoParams(FNpcKilledActorEvent, AActor* KilledActor, const FGameplayTag& LastHitType);
	
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetAttackRange_NpcCombat() const = 0;
	virtual FGameplayAttribute GetAttackRangeAttribute() const = 0;
	
	virtual bool Dodge(const FVector& DodgeLocation) = 0;
	virtual void CancelDodge() = 0;
	virtual bool IsDodgeActive() const = 0;
	
	virtual bool BlockAttack() = 0;
	virtual void CancelBlock() = 0;

	virtual void StartAttack() = 0;
	virtual void RequestNextAttack() = 0;
	virtual void CancelAttack() = 0;
	
	virtual void ChargeIn(float VerticalImpulseStrength, float ForwardImpulseStrength, const FVector& ToLocation) = 0;
	virtual bool IsChargeInActive() const = 0;
	virtual void CancelChargeIn() = 0;

	virtual double GetAttackPhaseEndTime() const = 0;
	virtual bool CanAttack() const = 0;
	virtual bool CanBlock() const = 0;
	
	virtual INpcThreat* GetActiveTarget() const = 0;
	
	mutable FNpcKilledActorEvent KilledActorEvent_NpcCombatant;
};
