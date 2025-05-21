// 

#pragma once

#include "CoreMinimal.h"
#include "EnvQueryGenerator_SmartObjects.h"
#include "EnvQueryGenerator_ActivitySmartObjects.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryGenerator_ActivitySmartObjects : public UEnvQueryGenerator
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_ActivitySmartObjects();

protected:
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	/** The context indicating the locations to be used as query origins */
	UPROPERTY(EditAnywhere, Category=Generator)
	TSubclassOf<UEnvQueryContext> QueryOriginContext;
	
	/** Determines whether only currently claimable slots are allowed */
	UPROPERTY(EditAnywhere, Category = Generator)
	bool bOnlyClaimable = true;
};
