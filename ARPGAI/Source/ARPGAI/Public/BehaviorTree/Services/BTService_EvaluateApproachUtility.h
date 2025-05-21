

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UObject/Object.h"
#include "BTService_EvaluateApproachUtility.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_EvaluateApproachUtility : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_EvaluateApproachUtility();
	virtual FString GetStaticDescription() const override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector StraightforwardUtilityBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ManeuverUtilityBBKey;

private:
	void EvaluateUtility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;
};
