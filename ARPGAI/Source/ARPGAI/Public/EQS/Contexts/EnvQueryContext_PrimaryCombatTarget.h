// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_PrimaryCombatTarget.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryContext_PrimaryCombatTarget : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;	
};
