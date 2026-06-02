// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_ValuableItems.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_ValuableItems : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery ActorTagsFilter;
};
