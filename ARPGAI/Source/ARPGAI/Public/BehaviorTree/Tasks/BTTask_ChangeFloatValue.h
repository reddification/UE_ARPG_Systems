// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChangeFloatValue.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_ChangeFloatValue : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ChangeFloatValue();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector FloatValueBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DeltaValue = 0.f;	
};
