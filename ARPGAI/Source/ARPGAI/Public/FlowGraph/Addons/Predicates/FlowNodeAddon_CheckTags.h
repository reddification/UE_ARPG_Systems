// 

#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "FlowNodeAddon_CheckTags.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UFlowNodeAddon_CheckTags : public UFlowNodeAddOn, public IFlowPredicateInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery QueryFilter;

#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
	
public: // IFlowPredicateInterface
	virtual bool EvaluatePredicate_Implementation() const override;
};
