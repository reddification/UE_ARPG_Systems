// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_ValidArea.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryTest_ValidArea : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_ValidArea();
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	UPROPERTY(EditDefaultsOnly)
	FAIDataProviderFloatValue AreaExtent;
};
