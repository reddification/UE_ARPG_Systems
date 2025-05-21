

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_IsAtLocation.h"
#include "BTDecorator_IsAtLocation_Enhanced.generated.h"

UCLASS()
class ARPGAI_API UBTDecorator_IsAtLocation_Enhanced : public UBTDecorator_IsAtLocation
{
	GENERATED_BODY()

private:
	struct FBTMemory_IsAtLocationEnhanced
	{
		bool bIsDecoratorActive = false;
	};
	
public:
	
	UBTDecorator_IsAtLocation_Enhanced(const FObjectInitializer& ObjectInitializer);
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_IsAtLocationEnhanced); };
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AcceptableRangeBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAddNavAgentRadius = true;
};