// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_RememberedTraits.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryTest_RememberedTraits : public UEnvQueryTest
{
	GENERATED_BODY()
	
public:
	UEnvQueryTest_RememberedTraits();
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	FGameplayTagQuery RememberedTraitsFilter;
};
