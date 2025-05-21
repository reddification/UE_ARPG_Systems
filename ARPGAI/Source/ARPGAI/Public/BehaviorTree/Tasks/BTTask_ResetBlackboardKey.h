

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UObject/Object.h"
#include "BTTask_ResetBlackboardKey.generated.h"

UCLASS()
class ARPGAI_API UBTTask_ResetBlackboardKey : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ResetBlackboardKey();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override; 

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector BBKeyToReset;
};
