// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CopyBlackboardValue.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_CopyBlackboardValue : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CopyBlackboardValue();
	virtual FString GetStaticDescription() const override;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector SourceBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector DestinationBBKey;

private:
	EBlackboardNotificationResult OnSourceChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};
