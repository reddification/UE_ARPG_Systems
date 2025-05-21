#pragma once

#include "BTDecorator_UtilityFunction.h"
#include "BTDecorator_UtilityBlackboard.generated.h"


class UBehaviorTree;

/**
* Blackboard based utility function.
* The associated node's utility value is specified via a blackboard key.
* The key must be of type Float or Integer.
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
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;


	/** get name of selected blackboard key */
	FName GetSelectedBlackboardKey() const;

	/**
	 * Returns the blackboard key that defines our utility.
	 */
	FBlackboardKeySelector GetSelectedBlackboardKeySelector() const;

	/**
	 * Notify about change in blackboard key.
	 * Our parent BTComposite_Utility binds and unbinds this for us: see BTComposite_Utility::WatchChildBlackboardKeys()
	 */
	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);

protected:
	/** blackboard key selector */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector UtilityValueKey;
	
	virtual float CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

FORCEINLINE FName UBTDecorator_UtilityBlackboard::GetSelectedBlackboardKey() const
{
	return UtilityValueKey.SelectedKeyName;
}

FORCEINLINE FBlackboardKeySelector UBTDecorator_UtilityBlackboard::GetSelectedBlackboardKeySelector() const
{
	return UtilityValueKey;
}
