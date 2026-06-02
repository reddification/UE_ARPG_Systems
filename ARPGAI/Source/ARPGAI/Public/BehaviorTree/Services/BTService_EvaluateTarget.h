#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_EvaluateTarget.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_EvaluateTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_EvaluateTarget();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutAttitudeBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutStaticThreatScoreBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutTargetTagsBBKey;
	
	// UPROPERTY(EditAnywhere)
	// FBlackboardKeySelector OutAttackRangeBBKey;
	//
	// UPROPERTY(EditAnywhere)
	// FBlackboardKeySelector OutNormalizedHealthBBKey;
	
private:
	EBlackboardNotificationResult OnTargetChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	void UpdatePropertiesInBlackboard(UBehaviorTreeComponent& BTComponent);
	void ClearBlackboard(UBlackboardComponent* Blackboard);
};
