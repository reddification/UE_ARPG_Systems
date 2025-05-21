#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_OptionalBlackboardTimeLimit.generated.h"

UCLASS(Category="Common")
class ARPGAI_API UBTDecorator_OptionalBlackboardTimeLimit : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_OptionalBlackboardTimeLimit : public FBTAuxiliaryMemory
	{
		bool bTimeLimitSet = false;
		bool bTimeLimitExpired = false;
	};
	
public:
	UBTDecorator_OptionalBlackboardTimeLimit();
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_OptionalBlackboardTimeLimit); }
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
protected:
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void OnNodeProcessed(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TimeLimitBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector IsIndefiniteBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bTreatTimeoutAsSuccess = true;

	// Dirty hack temp variable, see comments OnNodeDeactivation for explanation
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSaveRemainingGoalTime = false;
};