// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NpcHealingComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcHealingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual bool CanHeal() const { return false; }
	virtual bool Heal() { return false; }
	virtual void AbortHeal() { };
};
