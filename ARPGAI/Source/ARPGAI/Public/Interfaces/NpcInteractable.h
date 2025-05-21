// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionStarted(const FGameplayTagContainer& GrantedTags, bool bPermanent);

	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionEnded();
};
