// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcZone.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcZone : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcZone
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)	
	TArray<UShapeComponent*> GetZoneCollisionVolumes() const;
};
