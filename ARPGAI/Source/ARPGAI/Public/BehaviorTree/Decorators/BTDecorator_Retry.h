// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_Retry.generated.h"

/**
 * 
 */

UENUM()
enum class EBehaviorRetryMode : uint8
{
	Time = 0,
	Count = 1
};

UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_Retry : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_Retry
	{
		float TryUntilWorldTime = 0.f;
		int RetryCountsRemain = 0;
	};
	
public:
	UBTDecorator_Retry();
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_Retry); }
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBehaviorRetryMode RetryMode = EBehaviorRetryMode::Time;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float TimeLimitMin = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float TimeLimitMax = 30.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0, ClampMin = 0))
	int AttemptsMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0, ClampMin = 0))
	int AttemptsMax = 3;
};
