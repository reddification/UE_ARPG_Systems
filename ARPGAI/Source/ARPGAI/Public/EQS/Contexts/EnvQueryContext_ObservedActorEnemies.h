#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_ObservedActorEnemies.generated.h"

UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_ObservedActorEnemies : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag StoredActorTag;
};
