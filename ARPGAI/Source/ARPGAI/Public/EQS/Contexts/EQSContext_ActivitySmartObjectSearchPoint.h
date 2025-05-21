// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "UObject/Object.h"
#include "EQSContext_ActivitySmartObjectSearchPoint.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEQSContext_ActivitySmartObjectSearchPoint : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
