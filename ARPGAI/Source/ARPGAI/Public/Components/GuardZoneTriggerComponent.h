

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuardZoneTriggerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UGuardZoneTriggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool IsTriggeringZone() const;
	
private:
	bool bTriggersZone = true;
};
