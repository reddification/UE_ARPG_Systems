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
	virtual bool StartBlocking(const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayEventData* TriggerEventData) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void OnNpcFinishedBlocking();
};
