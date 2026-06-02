#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_UtilityFunction.generated.h"

/** 
 * Utility functions are responsible for providing a utility value for their associated node whenever the 
 * parent utility selector requests it.
 */
UCLASS(Abstract, HideCategories = (Condition))
class ARPGAI_API UBTDecorator_UtilityFunction : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_UtilityDecorator
	{
		bool bActive = false;
	};
	
public:
	UBTDecorator_UtilityFunction(const FObjectInitializer& ObjectInitializer);
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_UtilityDecorator); };
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	
	/** wrapper for node instancing: CalculateUtilityValue */
	float WrappedCalculateUtility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;
	bool IsBranchActive(const UBehaviorTreeComponent& OwnerComp) const;
	
protected:
	/** Calculates the utility value of the associated behavior node. */
	virtual float CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;
};
