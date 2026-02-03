// 

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_OtherCombatantLocations.generated.h"

enum class ENpcCombatRole : uint8;
/**
 * 
 */
UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_OtherCombatantLocations : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPredictLocations = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ENpcCombatRole> Roles;
};
