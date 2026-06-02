// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_SurfaceNormal.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryTest_SurfaceNormal : public UEnvQueryTest
{
	GENERATED_BODY()
	
public:
	UEnvQueryTest_SurfaceNormal();
	
protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionDetails() const override;
	
	UPROPERTY(EditDefaultsOnly, Category=Filter)
	FAIDataProviderFloatValue TraceDownDistanceParam;
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};
