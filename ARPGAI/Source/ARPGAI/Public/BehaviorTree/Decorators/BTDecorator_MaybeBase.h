

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_MaybeBase.generated.h"

UCLASS(Abstract)
class ARPGAI_API UBTDecorator_MaybeBase : public UBTDecorator
{
	GENERATED_BODY()

public:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	virtual float GetProbability(UBlackboardComponent* Blackboard) const;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHandledByWeightedRandom = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bForceSuccess = false;

	virtual void OnNodeProcessed(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult) override;
};
