// 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_AttackByMontageShapeSweep.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_AttackByMontageShapeSweep : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_AttackByMontageShapeSweep();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageGE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageSphereRadius = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName DamageOriginSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag OverlapCollisionEventTag;

	// In case hit actor has no GAS. Perhaps some destructible crates, fences, etc
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FallbackRawDamage = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDrawSweepDebug;

private:
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* ActiveMontageTask;

	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitOverlapTask;
	
	UFUNCTION()
	void DoOverlap(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnMontageInterrupted();

	FGameplayTag GetHitDirectionTag(const AActor* Actor, const FVector& ImpactPoint) const;
};
