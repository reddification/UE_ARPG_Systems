// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_Block.h"
#include "GameplayAbility_NpcBlock.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_NpcBlock : public UGameplayAbility_Block
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool StartBlocking(const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayEventData* TriggerEventData) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void OnAttackParried(UActorComponent* ActorComponent, const FHitResult& HitResult, const FVector& Vector) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FContextMontages> BackstepMontages;
	
private:
	void OnNpcFinishedBlocking();
};
