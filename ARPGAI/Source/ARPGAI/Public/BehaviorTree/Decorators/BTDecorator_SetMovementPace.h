// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_SetMovementPace.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_SetMovementPace : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_SetMovementPace : public FBTAuxiliaryMemory
	{
		FGameplayTag InitialMovementPaceType = FGameplayTag::EmptyTag;
	};
	
public:
	UBTDecorator_SetMovementPace();
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_SetMovementPace); };
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ActivationDelay = 0.f;
	
	UPROPERTY(EditAnywhere, meta=(Categories="AI.State.Pace"))
	FGameplayTag MovementPaceType = FGameplayTag::EmptyTag;
};
