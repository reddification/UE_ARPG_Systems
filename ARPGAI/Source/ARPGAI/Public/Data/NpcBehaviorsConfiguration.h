#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "NpcBehaviorsConfiguration.generated.h"

class UBehaviorEvaluatorConfig_Base;
class UBehaviorTree;
/**
 * 
 */
UCLASS()
class ARPGAI_API UNpcBehaviorsConfiguration : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBehaviorTree* BaseBehavior;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, UBehaviorTree*> DynamicBehaviors;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer DefaultBehaviorEvaluators;

	UFUNCTION(CallInEditor)
	void GenerateFormulasDescriptions();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<UBehaviorEvaluatorConfig_Base*> BehaviorEvaluators;
};
