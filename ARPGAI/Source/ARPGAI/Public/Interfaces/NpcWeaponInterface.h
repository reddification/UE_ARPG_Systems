// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcWeaponInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcWeaponInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool RequestWeaponReady_NPC(bool bSetReady) = 0;
	virtual void CancelWeaponReady_NPC(bool bSetReady) = 0;
	virtual void DropUnsheathedWeapon_NPC() = 0;
};
