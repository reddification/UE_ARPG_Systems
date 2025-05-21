// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_MeleeWeaponCombat.h"
#include "GameplayAbility_PlayerMovesetWeaponCombat.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_PlayerMovesetWeaponCombat : public UGameplayAbility_MeleeWeaponCombat
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	
	// virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	// 	FGameplayTagContainer* OptionalRelevantTags) const override;
	// virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	// 	const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EMeleeAttackType, TSubclassOf<UGameplayEffect>> AttackTypesCosts;

private:
	
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitForReactivateAttackRequest;

	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitForFeintRequest;

	UFUNCTION()
	void OnAttackReactivateRequested(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackFeintRequested(FGameplayEventData Payload);
};