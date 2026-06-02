// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_RequestBehaviorEvaluatorState.generated.h"

UCLASS(HideCategories=(FlowControl, Condition))
class ARPGAI_API UBTDecorator_RequestBehaviorEvaluatorState : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_RequestBehaviorEvaluatorState();
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	// Recommended to be unique in request stack, but in general - optional
	UPROPERTY(EditAnywhere)
	FName RequestId = FName("BTDecorator_RequestBehaviorEvaluatorState");
	
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer EvaluatorIds;
	
	UPROPERTY(EditAnywhere)
	bool bRequestRelevant = true;	
};
