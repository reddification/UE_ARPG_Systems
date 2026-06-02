#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTDecorator_CheckDirection.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_CheckDirection : public UBTDecorator
{
	GENERATED_BODY() 
	
public:
	UBTDecorator_CheckDirection();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetBBKey;
	
	UPROPERTY(EditAnywhere)
	FValueOrBBKey_Float ViewDirectionYawOffset = 0.f;
	
	// This is half angle. Compared with dot product
	UPROPERTY(EditAnywhere)
	FValueOrBBKey_Float AngleThreshold = 60.f;
	
private:
	EBlackboardNotificationResult OnTargetChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};
