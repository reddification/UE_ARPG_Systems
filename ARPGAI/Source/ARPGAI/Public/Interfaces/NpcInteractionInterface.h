// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcInteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcInteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void StartInteracting_NPC(AActor* SmartObjectActor, const struct FSmartObjectClaimHandle& SmartObjectClaimHandle) = 0;
	virtual void StopInteracting_NPC() = 0;
	virtual bool IsInteracting_NPC() const = 0;

};
