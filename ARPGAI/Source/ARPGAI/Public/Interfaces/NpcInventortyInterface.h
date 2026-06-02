// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcInventortyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcInventortyInterface : public UInterface
{
	GENERATED_BODY()
};

class ARPGAI_API INpcInventortyInterface
{
	GENERATED_BODY()

public:
	virtual void GiveMoney_NPC(int Gold) = 0;
	virtual bool LootContainer_NPC(AActor* Actor) = 0;
	virtual void StopLooting_NPC() = 0;
};
