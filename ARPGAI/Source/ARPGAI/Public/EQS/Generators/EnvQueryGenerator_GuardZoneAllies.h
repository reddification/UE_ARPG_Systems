

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ActorsOfClass.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_PerceivedActors.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "UObject/Object.h"
#include "EnvQueryGenerator_GuardZoneAllies.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Current Allies"))
class ARPGAI_API UEnvQueryGenerator_GuardZoneAllies : public UEnvQueryGenerator
{
	GENERATED_BODY()
public:
	UEnvQueryGenerator_GuardZoneAllies(const FObjectInitializer& ObjectInitializer);
	
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
protected:
	/** context */
	UPROPERTY(EditDefaultsOnly, Category = Generator)
	TSubclassOf<UEnvQueryContext> QueryContext;
};
