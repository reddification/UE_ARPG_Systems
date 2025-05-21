// 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "GameplayAbility_BaseCombat.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class COMBAT_API UGameplayAbility_BaseCombat : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_BaseCombat();
	
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
};
