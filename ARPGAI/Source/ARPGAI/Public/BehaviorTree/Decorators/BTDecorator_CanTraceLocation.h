// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTDecorator_CanTraceLocation.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_CanTraceLocation : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_CanTraceLocation();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TraceTargetBBKey;
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	
	UPROPERTY(EditAnywhere)
	FValueOrBBKey_Float TraceEndOffsetZ = 50.f; 
	
private:
	EBlackboardNotificationResult OnTraceTargetChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	
};
