// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Decorators/BTDecorator_Loop.h"
#include "BTDecorator_Repeat.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition,Decorator))
class ARPGAI_API UBTDecorator_Repeat : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_Repeat : public FBTLoopDecoratorMemory
	{
		float ActualRepeatDuration = 0.f;
	};
	
public:
	UBTDecorator_Repeat();
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_Repeat); };
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	float RepeatIntervalMin = 5.f;

	UPROPERTY(EditAnywhere)
	float RepeatIntervalMax = 10.f;
};
