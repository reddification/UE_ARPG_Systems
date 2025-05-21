

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsCircumventionRequired.generated.h"

UCLASS()
class ARPGAI_API UBTDecorator_IsCircumventionRequired : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_IsCircumventionRequired();
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetActorBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector DestinationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = -1.f, UIMax = 0.f, ClampMin = -1.f, ClampMax = 0.f))
	float DotProductThreshold = -0.4f;

private:
	EBlackboardNotificationResult OnDestinationChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey KeyId);
};
