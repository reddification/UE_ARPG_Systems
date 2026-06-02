#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_Repeat.generated.h"

UCLASS(HideCategories=(FlowControl,Condition,Decorator))
class ARPGAI_API UBTDecorator_Repeat : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_Repeat
	{
		double TimeStarted = 0.;
		float ActualRepeatDuration = 0.f;
		int32 SearchId = -1;
	};
	
public:
	UBTDecorator_Repeat();
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_Repeat); };
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual FString GetStaticDescription() const override;
	
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

	
protected:
	UPROPERTY(EditAnywhere)
	float RepeatIntervalMin = 5.f;

	UPROPERTY(EditAnywhere)
	float RepeatIntervalMax = 10.f;
};
