// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UObject/Object.h"
#include "BTDecorator_ActivateBehavior.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_ActivateBehavior : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_ActivateBehavior();
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	UPROPERTY(EditAnywhere)
	FGameplayTag BehaviorTag;
};
