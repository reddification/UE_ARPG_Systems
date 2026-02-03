// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_HitReact.h"
#include "GameplayAbility_NpcHitReact.generated.h"

class UAbilityTask_PlayAnimAndWait;
/**
 * 
 */
UCLASS()
class COMBAT_API UGameplayAbility_NpcHitReact : public UGameplayAbility_HitReact
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FContextMontages> BackstepMontageOptions;
};
