// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcMovementNoiseProducer.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcMovementNoiseProducer : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcMovementNoiseProducer
{
	GENERATED_BODY()

public:
	virtual bool CanBeHeardMoving_NPC() = 0;
	virtual float GetCarriedWeight_NPC() = 0;
	virtual float GetSpeed_NPC() = 0;
	virtual float GetDexterity_NPC() = 0;
};
