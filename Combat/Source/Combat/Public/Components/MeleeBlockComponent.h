// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/CombatAnimInstance.h"
#include "Interfaces/ICombatant.h"
#include "MeleeBlockComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UMeleeBlockComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DELEGATE_ThreeParams(FOnAttackParriedEvent, UActorComponent* OtherComponent, const FHitResult& SweepResult, const FVector& ParryDirection)
	DECLARE_DELEGATE_OneParam(FOnAttackBlockedEvent, float BlockConsumption)
	
public:
	UMeleeBlockComponent();
	virtual void StartBlocking();
	virtual void StopBlocking();
	EBlockResult BlockAttack(const FVector& AttackDirection, float AttackerStrength) const;
	bool IsBlocking() const { return bRegisteringBlock; };

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	mutable FOnAttackParriedEvent OnAttackParriedEvent;
	mutable FOnAttackBlockedEvent OnAttackBlockedEvent;

protected:
	virtual FVector2D GetBlockInput(float DeltaTime) const { unimplemented(); return FVector2D::ZeroVector; };
	virtual void AddBlockInput(const FVector2D& BlockInput, float DeltaTime);
	FVector GetIncomingAttackDirection(const AActor* AttackingActor, EMeleeAttackType IncomingAttackType);
	FVector2D GetDesiredBlockVector(EMeleeAttackType IncomingAttackType) const;

	UPROPERTY()
	TScriptInterface<ICombatant> OwnerCombatant = nullptr;
	
	float BlockInputAccumulationScale = 0.1f;
	FVector2D AccumulatedBlock = FVector2D::ZeroVector;

private:
	bool bRegisteringBlock = false;
	float CollinearBlockInputsDotProductThreshold = 0.85f;
	float StrongBlockActivationThreshold = 0.8f;
	float BlockStrength = 0.f;
	float BlockStrengthDecayRate = 3.f;
	float CurrentDecayDelay = 0.f;
	float DecayDelay = 0.5f;
	float BlockStrengthAccumulationScale = 2.f;
	float MinHeldBlockStrength = 0.25f;
	float BlockStrengthToParry = 0.5f;
	float AttackerStrengthScaleWhenHitBlock = 0.25f;
	
	UPROPERTY()
	TScriptInterface<ICombatAnimInstance> CombatAnimInstance = nullptr;
};
