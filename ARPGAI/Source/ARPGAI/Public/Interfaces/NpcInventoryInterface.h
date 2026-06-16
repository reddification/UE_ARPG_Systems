// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcInventoryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

class ARPGAI_API INpcInventoryInterface
{
	GENERATED_BODY()

public:
	virtual void GiveMoney_NPC(int Gold) = 0;
	virtual bool LootContainer_NPC(AActor* Actor) = 0;
	virtual void StopLooting_NPC() = 0;
	virtual bool EquipBestWeapon_NPC() = 0;
};
