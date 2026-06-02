// 

#pragma once

#include "CoreMinimal.h"
#include "BTDecorator_MaybeBlackboardBase.h"
#include "BTDecorator_MaybeCurve.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_MaybeCurve : public UBTDecorator_MaybeBlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTDecorator_MaybeCurve();
	virtual float GetProbability(UBlackboardComponent* Blackboard) const override;
	
protected:
	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve ProbabilityCurve;
};
