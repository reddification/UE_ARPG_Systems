

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ObserveIntruders.generated.h"

class ALyraCharacter;

UCLASS()
class ARPGAI_API UBTService_ObserveIntruders : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_ObserveIntruders();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float IntruderAlertRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutCurrentTargetBBKey;

private:
	APawn* GetClosestIntruder(AAIController* AIController) const;
};
