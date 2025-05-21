// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "SmartObjectUserInteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class SMARTOBJECTINTERACTION_API USmartObjectUserInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SMARTOBJECTINTERACTION_API ISmartObjectUserInteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool StartUsingSmartObject() = 0;
	virtual bool StartUsingSmartObject(const FGameplayTag& GestureTag) = 0;
	virtual void GrantSmartObjectUsageTags(const FGameplayTagContainer& GameplayTags) = 0;
	virtual void StopUsingSmartObject() = 0;
	virtual void RemoveSmartObjectInteractionTags(const FGameplayTagContainer& GrantedTags) = 0;
};
