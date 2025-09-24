// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanSeeActor.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_CanSeeActor : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CanSeeActor();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ActorBBKey;
};
