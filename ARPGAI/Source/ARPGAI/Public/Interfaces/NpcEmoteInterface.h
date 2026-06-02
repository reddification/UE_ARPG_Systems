// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "NpcEmoteInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcEmoteInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcEmoteInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool PerformGesture_NPC(const FGameplayTag& GestureTag) = 0;
	virtual void StopGesture_NPC() = 0;
	virtual bool SayPhrase_NPC(const FGameplayTag& PhraseTag) = 0;
};
