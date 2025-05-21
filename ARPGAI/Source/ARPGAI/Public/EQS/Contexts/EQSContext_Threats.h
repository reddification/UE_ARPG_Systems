

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "UObject/Object.h"
#include "EQSContext_Threats.generated.h"

UCLASS()
class ARPGAI_API UEQSContext_Threats : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
