// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_ExpectedEnemiesLocations.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_ExpectedEnemiesLocations : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAll = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="!bAll"))
	int TopNRecent = 3;
	
	// if filtered, don't consider memories that are not updated more than this time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="!bAll", UIMin = 0, ClampMin = 0))
	double RelevancyTimeThreshold = 30.f;
};
