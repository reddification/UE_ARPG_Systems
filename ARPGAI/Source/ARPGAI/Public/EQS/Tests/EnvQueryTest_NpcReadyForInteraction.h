

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "UObject/Object.h"
#include "EnvQueryTest_NpcReadyForInteraction.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryTest_NpcReadyForInteraction : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_NpcReadyForInteraction(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionDetails() const override;
};
