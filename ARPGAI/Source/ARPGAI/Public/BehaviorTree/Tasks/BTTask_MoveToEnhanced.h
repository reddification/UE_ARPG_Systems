

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MoveToEnhanced.generated.h"

UCLASS()
class ARPGAI_API UBTTask_MoveToEnhanced : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_MoveToEnhanced(const FObjectInitializer& ObjectInitializer);
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ApproachAcceptanceRadiusBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ApproachChangeDistanceThresholdBBKey;
};
