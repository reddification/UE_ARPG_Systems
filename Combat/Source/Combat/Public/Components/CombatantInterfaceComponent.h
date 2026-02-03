// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatantInterfaceComponent.generated.h"


UCLASS(Abstract)
class COMBAT_API UCombatantInterfaceComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	virtual void OnBackdashStarted() { };
	virtual void OnBackdashFinished(bool bSuccess) { };
};
