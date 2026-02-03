// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_BonesOnCharacter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_BonesOnCharacter : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag CharacterStoredDataKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> BonesNames;
};
