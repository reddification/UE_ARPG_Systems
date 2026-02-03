// 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_AttackByMontageShapeSweep.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_PlayMontageAndWait;

USTRUCT(BlueprintType)
struct FAttackByMontageData
{
	GENERATED_BODY()

	// different damage types: puncturing, cutting, bludgeoning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> Damages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PoiseDamage = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamageSphereRadius = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName DamageOriginSocketName;
};

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
	TArray<FAttackByMontageData> Attacks;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UAnimMontage*> AttackMontages_Obsolete;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageGE;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	// float DamageSphereRadius = 25.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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
	
	int ActiveAttackIndex = 0;
};
