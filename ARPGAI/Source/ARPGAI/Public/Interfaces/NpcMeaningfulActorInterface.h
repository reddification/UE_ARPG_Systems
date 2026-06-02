// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcMeaningfulActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcMeaningfulActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcMeaningfulActorInterface
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_TwoParams(FNpcMeaningfulActorDestroyedEvent, AActor* Actor, AActor* Instigator)
	
public:
	FNpcMeaningfulActorDestroyedEvent NpcMeaningfulActorDestroyedEvent;
};
