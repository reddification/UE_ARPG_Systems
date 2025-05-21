#pragma once

#include "CoreMinimal.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "UObject/Object.h"
#include "EnvQueryTest_DoesPathExist.generated.h"

UCLASS()
class ARPGAI_API UEnvQueryTest_DoesPathExist : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_DoesPathExist();
	
	/** context: other end of pathfinding test */
	UPROPERTY(EditDefaultsOnly, Category=Pathfinding)
	TSubclassOf<UEnvQueryContext> Context;

	/** pathfinding direction */
	UPROPERTY(EditDefaultsOnly, Category=Pathfinding)
	FAIDataProviderBoolValue PathFromContext;
	
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;

protected:

	bool TestPathFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const;
	bool TestPathTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const;
	
	ANavigationData* FindNavigationData(UNavigationSystemV1& NavSys, UObject* Owner) const;
};
