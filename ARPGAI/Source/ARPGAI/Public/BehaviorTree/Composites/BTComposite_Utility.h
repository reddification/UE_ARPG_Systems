#pragma once

#include "BehaviorTree/BTCompositeNode.h"
#include "BTComposite_Utility.generated.h"

class UBTDecorator_UtilityFunction;

#define MAX_UTILITY_NODES 16

struct FIndexedUtilityValue
{
	FIndexedUtilityValue(int32 ChildIndex, float Utility)
	{
		ChildIdx = ChildIndex;
		UtilityScore = Utility;
	}
	
	int32 ChildIdx;
	float UtilityScore;

	bool operator < (const FIndexedUtilityValue & Other) const
	{
		return UtilityScore > Other.UtilityScore; // 29.11.2024 @AK: we rely on stable sorting so that items with equal utility aren't swapped back and forth
		// return (UtilityScore > Other.UtilityScore) || (UtilityScore == Other.UtilityScore && ChildIdx < Other.ChildIdx);
	}

	void SetInvalid();
};


struct FBTUtilityMemory : public FBTCompositeMemory
{
	FIndexedUtilityValue ExecutionUtilityOrdering[MAX_UTILITY_NODES];
	uint8 ActualUtilityNodesCount = 0;
	bool bUtilityChanged = false;
};

UCLASS()
class ARPGAI_API UBTComposite_Utility: public UBTCompositeNode
{
	GENERATED_BODY()

public:
	UBTComposite_Utility(const FObjectInitializer& ObjectInitializer);
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	virtual int32 GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const override;
	void Reevaluate(UBehaviorTreeComponent* BehaviorTreeComponent);
	
protected:
	void EvaluateUtilityScores(FBehaviorTreeSearchData& SearchData, FBTUtilityMemory* UtilityMemory) const;
	const UBTDecorator_UtilityFunction* FindChildUtilityDecorator(int32 ChildIndex) const;
	
	/**
	 * Gets all the scores via EvaluateUtilityScores, then sorts them for selection.
	 */
	virtual void OrderUtilityScores(FBehaviorTreeSearchData& SearchData) const;

	/**
	 * Called when start enters this node.
	 * 
	 * Evaluate utilities,
	 * and set up child BTDecorator_UtilityBlackboards to monitor their blackboard keys so that they can abort and we can reevaluate if they change.
	 */
	virtual void NotifyNodeActivation(FBehaviorTreeSearchData& SearchData) const override;
	
	/**
	 * Called when start leaves this node.
	 * 
	 * Stop monitoring for blackboard key changes.
	 */
	virtual void NotifyNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult) const override;

	/**
	 * Set up child BTDecorator_UtilityBlackboard to monitor their blackboard keys so that they can abort and reevaluate if they change.
	 * We have to do this here because we're always active when our children are executing, but non-executing children aren't active when other children are,
	 * so they can't register and unregister in OnBecomeRelevant and OnCeaseRelevant because they're always CeaseRelevant unless they're executing.
	 * Additionally we can't register as the observer (i.e. do the whole thing in this node) as all composite (us) node functions are const, so we can't pass `this`
	 * to the observer function.
	 * 
	 * So: when we're entered, we iterate through our children and  tell the blackboard to alert our child blackboard utility decorators when their value changes.
	 * Then when a value changes the matching decorator requests execution, which triggers our NotifyNodeActivation(), which reevaluates all utilities.
	 */
	virtual void WatchChildBlackboardKeys(FBehaviorTreeSearchData& SearchData) const;

	/**
	 * Stop monitoring the blackboard keys of all our BTDecorator_UtilityBlackboard children.
	 */
	virtual void UnwatchChildBlackboardKeys(FBehaviorTreeSearchData& SearchData) const;

	// Makes the utility branch to act looped
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility")
	bool bReevaluateOnBranchSucceeded = true;

protected:
	// @AK 12.09.2024 fucking UE 5.4 gone insane and gives some obscure errors which are resolved by overriding some virtual methods with just Super:: invocation
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
};

