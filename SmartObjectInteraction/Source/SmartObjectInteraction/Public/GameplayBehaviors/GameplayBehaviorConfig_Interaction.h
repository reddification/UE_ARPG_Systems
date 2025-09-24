#pragma once

#include "CoreMinimal.h"
#include "GameplayBehaviorConfig.h"
#include "GameplayInteractionFunctionBase.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "GameplayBehaviorConfig_Interaction.generated.h"

UCLASS()
class SMARTOBJECTINTERACTION_API UGameplayBehaviorConfig_Interaction : public UGameplayBehaviorConfig
{
	GENERATED_BODY()

public:
	// 29.09.2024 @AK:TODO implement "Grant tags for game time hours" option 
	
	UPROPERTY(EditAnywhere)
	FGameplayTag GestureTag;
	
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer GrantTagsToUserOnStartUsing;

	UPROPERTY(EditAnywhere)
	bool bSmartObjectUserTagsPermanent = false;

	UPROPERTY(EditAnywhere)
	bool bSmartObjectUserChooseSingleRandomGrantedTag = false;
	
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer GrantTagsToSmartObjectActorOnStartUsing;

	UPROPERTY(EditAnywhere)
	bool bSmartObjectActorTagsPermanent = false;

	UPROPERTY(EditAnywhere)
	bool bSmartObjectActorChooseSingleRandomGrantedTag = false;

	UPROPERTY(EditAnywhere)
	TArray<TInstancedStruct<FGameplayInteractionFunctionBase>> InteractionFunctions;	
};
