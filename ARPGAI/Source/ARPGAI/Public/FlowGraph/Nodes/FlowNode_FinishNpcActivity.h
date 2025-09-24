// 

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNodeStateful.h"
#include "FlowNode_FinishNpcActivity.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UFlowNode_FinishNpcActivity : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_FinishNpcActivity(const FObjectInitializer& ObjectInitializer);
	virtual void ExecuteInput(const FName& PinName) override;
};
