

#pragma once

#include "CoreMinimal.h"
#include "BTDecorator_MaybeBase.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_MaybeBlackboardBase.generated.h"

UCLASS()
class ARPGAI_API UBTDecorator_MaybeBlackboardBase : public UBTDecorator_MaybeBase
{
	GENERATED_BODY()

	friend class UBTComposite_WeightedRandom;
	
public:
	UBTDecorator_MaybeBlackboardBase();
	virtual FString GetStaticDescription() const override;
	virtual float GetProbability(UBlackboardComponent* Blackboard) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ProbabilityBBKey;
};
