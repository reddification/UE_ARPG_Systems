// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "NpcValueableItemInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcValueableItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcValueableItemInterface
{
	GENERATED_BODY()
	
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual const FGameplayTag& GetItemTag_NPC() const = 0;
	virtual bool CanPickUp_NPC(AActor* Instigator) const = 0;
	virtual float GetValue_NPC() const = 0;
};
