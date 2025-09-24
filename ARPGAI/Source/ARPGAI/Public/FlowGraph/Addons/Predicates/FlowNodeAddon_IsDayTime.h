// 

#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "FlowNodeAddon_IsDayTime.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UFlowNodeAddon_IsDayTime : public UFlowNodeAddOn, public IFlowPredicateInterface
{
	GENERATED_BODY()

public:
	virtual bool EvaluatePredicate_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer DayTimes;

#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
};
