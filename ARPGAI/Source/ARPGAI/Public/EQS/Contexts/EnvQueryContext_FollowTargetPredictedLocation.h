// 

#pragma once

#include "CoreMinimal.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_FollowTargetPredictedLocation.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryContext_FollowTargetPredictedLocation : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UEnvQueryContext_FollowTargetPredictedLocation();
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

	UPROPERTY(EditAnywhere)
	FAIDataProviderFloatValue PredictionTimeParam;
};
