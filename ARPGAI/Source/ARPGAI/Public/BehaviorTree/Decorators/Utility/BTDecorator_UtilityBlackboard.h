#pragma once

#include "BTDecorator_UtilityFunction.h"
#include "BTDecorator_UtilityBlackboard.generated.h"


class UBehaviorTree;

/**
* Blackboard based utility function.
* The associated node's utility value is specified via a blackboard key.
* The key must be of type Float.
* When the value changes, even if we're not currently executing,
* our parent Utility composite node will reevaluate all utilities and select again.
*/
UCLASS(Meta = (DisplayName = "Blackboard Utility", Category = "Utility Functions"), HideCategories=FlowControl)
class ARPGAI_API UBTDecorator_UtilityBlackboard : public UBTDecorator_UtilityFunction
{
	GENERATED_UCLASS_BODY()

public:
	/** initialize any asset related data */
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual FString GetStaticDescription() const override;
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;

	/** get name of selected blackboard key */
	FORCEINLINE const FName& GetSelectedBlackboardKeyName() const { return UtilityValueKey.SelectedKeyName; }
	FORCEINLINE const FBlackboardKeySelector& GetSelectedBlackboardKeySelector() const { return UtilityValueKey; }
	FORCEINLINE bool IsSupressesPrevious() const { return bSuppressPrevious; }

protected:
	/** blackboard key selector */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector UtilityValueKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	bool bSuppressPrevious = true;
	
	virtual float CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};