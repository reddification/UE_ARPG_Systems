// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTargetReachable.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_UpdateTargetReachable : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTMemory_UpdateTargetReachable
	{
		uint8 ConsequitiveUnreachableChecks = 0;
	};
	
public:
	UBTService_UpdateTargetReachable();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_UpdateTargetReachable); };
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutIsTargetUnreachableBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseHierarchicalPathfinding = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 MinUnreachableChecksCount = 3;

private:
	EBlackboardNotificationResult OnTargetChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};
