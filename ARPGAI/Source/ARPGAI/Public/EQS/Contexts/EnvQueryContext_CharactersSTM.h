// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/NpcMemoryDataTypes.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_CharactersSTM.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_CharactersSTM : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer CharacterIdFilter;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer AttitudeFilter;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery CharacterStateFilter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TOptional<bool> MustBeAliveStateFilter;
	
	// check if "detections source has key" == value
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EDetectionSource, bool> DetectionSourcesFilter;
};
