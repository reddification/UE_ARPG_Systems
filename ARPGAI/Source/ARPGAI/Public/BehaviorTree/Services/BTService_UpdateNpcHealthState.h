#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateNpcHealthState.generated.h"

UCLASS()
class ARPGAI_API UBTService_UpdateNpcHealthState : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_UpdateNpcHealthState();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OptionalCurrentTargetBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutNormalizedHealthBBKey;
	
	// Normalized, short term
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutIndividualAccumulatedDamageBBKey;
	
	// Normalized, short term
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutTotalAccumulatedDamageBBKey;
	
private:
	void UpdateValues(UBehaviorTreeComponent& OwnerComp);
};
