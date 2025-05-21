// 

#pragma once

#include "CoreMinimal.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "EnvQueryGenerator_RoamingZone.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryGenerator_RoamingZone : public UEnvQueryGenerator_ProjectedPoints
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_RoamingZone(const FObjectInitializer& ObjectInitializer);

	/** generation density */
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	FAIDataProviderFloatValue SpaceBetween;

	/** generation density */
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	FAIDataProviderFloatValue ExtentScale;
	
	/** context */
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	TSubclassOf<UEnvQueryContext> GenerateAround;

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
};
