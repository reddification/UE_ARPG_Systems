#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "UObject/Object.h"
#include "EnvQueryGenerator_ActorsFromContext.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Actors from context"))
class ARPGAI_API UEnvQueryGenerator_ActorsFromContext : public UEnvQueryGenerator
{
	GENERATED_BODY()
public:
	UEnvQueryGenerator_ActorsFromContext(const FObjectInitializer& ObjectInitializer);
	
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
protected:
	/** context */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<UEnvQueryContext> QueryContext;
};
