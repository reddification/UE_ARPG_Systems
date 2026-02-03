// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_HitReactBase.h"
#include "GameplayAbility_Backdash.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_Backdash : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGameplayAbility_Backdash();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FContextMontages> MontageOptions;
	
private:
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;

	UFUNCTION()
	void OnBackdashMontageEnded();

	UFUNCTION()
	void OnBackdashMontageInterrupted();
	
	UFUNCTION()
	void OnBackdashMontageCancelled();
};
