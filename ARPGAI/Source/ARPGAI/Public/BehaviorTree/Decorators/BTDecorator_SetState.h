// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BTDecorator_SetState.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_SetState : public UBTDecorator_Blackboard
{
	GENERATED_BODY()

private:
	struct FBTMemory_SetState : public FBTAuxiliaryMemory
	{
		bool bStateApplied = false;
		bool bDelayFinished = false;
	};
	
public:
	UBTDecorator_SetState();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_SetState); };
	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID) override;

protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.State"))
	FGameplayTag NewStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	bool bUseCondition = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="SetByCaller"))
	TMap<FGameplayTag, float> SetByCallerParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ActivationDelay = 0.f;
};
