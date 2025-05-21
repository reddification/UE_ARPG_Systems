

#pragma once

#include "CoreMinimal.h"
#include "BTDecorator_MaybeBase.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_MaybeConst.generated.h"

UCLASS()
class ARPGAI_API UBTDecorator_MaybeConst : public UBTDecorator_MaybeBase
{
	GENERATED_BODY()

public:
	UBTDecorator_MaybeConst();
	virtual float GetProbability(UBlackboardComponent* Blackboard) const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f))
	float Probability = 0.5;
};
