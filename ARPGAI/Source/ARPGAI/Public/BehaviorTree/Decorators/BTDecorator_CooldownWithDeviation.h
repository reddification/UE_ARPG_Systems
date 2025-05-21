

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Cooldown.h"
#include "UObject/Object.h"
#include "BTDecorator_CooldownWithDeviation.generated.h"

struct FBTCooldownWithDeviationMemory : public FBTCooldownDecoratorMemory
{
	float ActualCoolDownTime = 0.f;
};

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_CooldownWithDeviation : public UBTDecorator_Cooldown
{
	GENERATED_BODY()

public:
	UBTDecorator_CooldownWithDeviation();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity,
		TArray<FString>& Values) const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	float DeviationTime = 0.f;
};
