#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_ClosestPointToArea.generated.h"

UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_ClosestPointToArea : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
	
	// if not empty - only areas of these categories are considered
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer NpcAreasCategoryFilter;
	
	// If empty - querier pawn location is used 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag StartPointEQSParam;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AreaExtent = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseHierarchicalPathfinding = false;
	
private:
	FVector GetStartLocation(APawn* Pawn) const;
};
