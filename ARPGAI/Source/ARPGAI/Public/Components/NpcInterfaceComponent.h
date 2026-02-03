// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NpcInterfaceComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcInterfaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual bool Backdash() { return false; }
	virtual float GetBaseMoveSpeed();
};
