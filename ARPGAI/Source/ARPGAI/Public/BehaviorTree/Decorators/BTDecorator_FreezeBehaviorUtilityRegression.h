#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_FreezeBehaviorUtilityRegression.generated.h"

UCLASS()
class ARPGAI_API UBTDecorator_FreezeBehaviorUtilityRegression : public UBTDecorator
{
	GENERATED_BODY()
	
private:
	struct FBTMemory_FreezeBehaviorUtilityRegression
	{
		bool bFreezeApplied = false;
	};
	
public:
	UBTDecorator_FreezeBehaviorUtilityRegression();
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_FreezeBehaviorUtilityRegression); };
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag BehaviorId;
	
	// if <= 0 -> Freeze indefinitely. Otherwise - freeze for this amount of time in real world seconds, 
	// but on decorator deactivation BE will get unfrozen anyway 
	UPROPERTY(EditAnywhere)
	float ForDuration = 0.f;
};
