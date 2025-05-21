// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/SmartObjectOwnerInteractionComponent.h"
#include "UObject/Interface.h"
#include "SmartObjectOwnerInteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class SMARTOBJECTINTERACTION_API USmartObjectOwnerInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SMARTOBJECTINTERACTION_API ISmartObjectOwnerInteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionStarted();

	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionEnded();

	UFUNCTION(BlueprintNativeEvent)
	void HandleInteractionEvent(const FGameplayTag& InteractionEventTag);

	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionSuccess();

	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionAbort();
	
	virtual void GrantSmartObjectUsageTags(const FGameplayTagContainer& GameplayTags) = 0;
	virtual void RemoveSmartObjectInteractionTags(const FGameplayTagContainer& GrantedTags) = 0;
};
