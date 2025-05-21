// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_SquadLeaderPredictedPosition.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryContext_SquadLeaderPredictedPosition : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
