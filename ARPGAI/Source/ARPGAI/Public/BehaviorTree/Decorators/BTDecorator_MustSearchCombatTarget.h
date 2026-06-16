#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_MustSearchCombatTarget.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_MustSearchCombatTarget : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_MustSearchCombatTarget();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	// if NOT territorial AND target last seen location is less then this value -> must search 
	UPROPERTY(EditAnywhere)
	float DistanceThreshold = 1000.f;
	
	// if territorial and target last seen location is within territory with this extent -> must search
	UPROPERTY(EditAnywhere)
	float TerritoryExtent = 300.f;
};
