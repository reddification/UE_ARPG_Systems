#pragma once

#include "CoreMinimal.h"
#include "GameplayInteractionFunctionBase.generated.h"

USTRUCT()
struct SMARTOBJECTINTERACTION_API FGameplayInteractionFunctionBase
{
	GENERATED_BODY()

public:
	virtual ~FGameplayInteractionFunctionBase() = default;
	
	virtual void OnStart(AActor* SmartObjectUser, AActor* SmartObjectOwner) const {};
	virtual void OnEnd(AActor* SmartObjectUser, AActor* SmartObjectOwner) const {};
};
