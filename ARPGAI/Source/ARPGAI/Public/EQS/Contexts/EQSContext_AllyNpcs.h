#pragma once

#include "CoreMinimal.h"
#include "Data/AiDataTypes.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "UObject/Object.h"
#include "EQSContext_AllyNpcs.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARPGAI_API UEQSContext_AllyNpcs : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECharacterQueryMode CharacterQueryMode = ECharacterQueryMode::AliveOnly;
};
