#pragma once

#include "BehaviorTree/BTCompositeNode.h"
#include "BTComposite_Utility.generated.h"

class UBTDecorator_UtilityBlackboard;
class UEnhancedBehaviorTreeComponent;
class UBTDecorator_UtilityFunction;

#define MAX_UTILITY_NODES 16

struct FIndexedUtilityValue
{
	int32 ChildIdx = 0;
	float UtilityScore = 0.f;
	TWeakObjectPtr<const UBTDecorator_UtilityFunction> BaseUtilityDecorator = nullptr;
	TWeakObjectPtr<const UBTDecorator_UtilityBlackboard> BlackboardUtilityDecorator = nullptr;

	FBlackboard::FKey GetBlackboardKey() const;
	FName GetBlackboardKeyName() const;
	
	bool operator < (const FIndexedUtilityValue & Other) const
	{
		// 29.11.2024 @AK: we rely on stable sorting so that items with equal utility aren't swapped back and forth
		return UtilityScore > Other.UtilityScore;
		// return (UtilityScore > Other.UtilityScore) || (UtilityScore == Other.UtilityScore && ChildIdx < Other.ChildIdx);
	}

	void SetInvalid();
};

UENUM()
enum class EUtilityNodeCompletedBehavior : uint8
{
	Finish,
	Next,
	Repeat
};

struct FBTUtilityMemory : public FBTCompositeMemory
{
	FIndexedUtilityValue ExecutionUtilityOrdering[MAX_UTILITY_NODES];
	uint8 ActualUtilityNodesCount = 0;
	bool bUtilityReevaluated = false;
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
	void OnUtilityChanged(UBehaviorTreeComponent& BehaviorComp, FBlackboard::FKey, uint8 ChildIndex) const;
	
protected:
	virtual void NotifyNodeActivation(FBehaviorTreeSearchData& SearchData) const override;
	virtual void NotifyNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult) const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility")
	EUtilityNodeCompletedBehavior OnSuccessBehavior = EUtilityNodeCompletedBehavior::Repeat;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility")
	EUtilityNodeCompletedBehavior OnFailureBehavior = EUtilityNodeCompletedBehavior::Finish;

private:
	void WatchChildBlackboardKeys_Old(FBehaviorTreeSearchData& SearchData) const;
	void UnwatchChildBlackboardKeys_Old(FBehaviorTreeSearchData& SearchData) const;
	EBlackboardNotificationResult OnUtilityChanged_Old(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key, uint8 ChildIndex);
	void EvaluateUtilityExecution(UBehaviorTreeComponent& BehaviorComp, FBTUtilityMemory* BTUtilityMemory,
	                              uint8 ChildIndex) const;
	void UpdateUtilityScores(FBTUtilityMemory* UtilityMemory, const UBlackboardComponent* Blackboard) const;
	void InitializeUtilityScores(FBehaviorTreeSearchData& SearchData, FBTUtilityMemory* UtilityMemory) const;
	const UBTDecorator_UtilityFunction* FindChildUtilityDecorator(int32 ChildIndex, UObject* LogOwner) const;
	
	void RegisterUtilityObserver(UEnhancedBehaviorTreeComponent* EnhancedBTComponent, const FBTUtilityMemory* BTMemory) const;
	void UnregisterUtilityObserver(UEnhancedBehaviorTreeComponent* EnhancedBTComponent, const FBTUtilityMemory* BTMemory) const;
	
};

