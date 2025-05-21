// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvQueryGenerator_ConversationPartners.generated.h"

struct FGameplayTagQuery;
/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryGenerator_ConversationPartners : public UEnvQueryGenerator
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_ConversationPartners();
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

protected:
	UPROPERTY(EditDefaultsOnly)
	FAIDataProviderFloatValue SearchRangeValue;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagQuery GameplayTagQuery;
};
