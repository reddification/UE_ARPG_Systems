// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcPerceptionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcPerceptionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcPerceptionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetHearingLoudnessThreshold_NPC() const = 0;
};
