// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectComponent.h"
#include "GameplayTagContainer.h"
#include "SetMovementPaceGameplayEffectComponent.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API USetMovementPaceGameplayEffectComponent : public UGameplayEffectComponent
{
	GENERATED_BODY()

protected:
	virtual bool OnActiveGameplayEffectAdded(FActiveGameplayEffectsContainer& ActiveGEContainer, FActiveGameplayEffect& ActiveGE) const override;
	virtual void OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag PaceTag;
	
private:
	void OnActiveGameplayEffectRemoved(const struct FGameplayEffectRemovalInfo& RemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer, FGameplayTag
	                                   InitialPaceType) const; 
};
