// 

#pragma once

#include "CoreMinimal.h"
#include "DebugDataTypes.h"
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
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	// for some weird, not reproduceable reasons, sometimes NPCs stuck in this ability. I tried to reproduce it but I just can't. 
	// But sometimes it still happens and it blocks NPCs attacking and parrying abilities. So fuck this shit, I will ensure NPCs leave HitReact state 
	FTimerHandle AutoCorrectionTimer;
	void AutoCorrectEndAbility();
};
