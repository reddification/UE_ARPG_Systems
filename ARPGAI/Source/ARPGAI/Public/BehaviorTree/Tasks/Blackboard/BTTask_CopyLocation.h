#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CopyLocation.generated.h"

UCLASS(Category="Blackboard")
class ARPGAI_API UBTTask_CopyLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_CopyLocation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector SourceBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutputBBKey;
};
