// 


#include "FlowGraph/Nodes/FlowNode_FinishNpcActivity.h"

#include "Components/Controller/NpcFlowComponent.h"

UFlowNode_FinishNpcActivity::UFlowNode_FinishNpcActivity(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
	Category = TEXT("NPC");
	NodeDisplayStyle = FlowNodeStyle::InOut;
#endif

	OutputPins = {};
	AllowedSignalModes = {EFlowSignalMode::Enabled, EFlowSignalMode::Disabled};
}

void UFlowNode_FinishNpcActivity::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	auto AIController = TryGetRootFlowActorOwner();
	if (auto NpcFlowComponent = AIController->FindComponentByClass<UNpcFlowComponent>())
		NpcFlowComponent->FinishActivity();
	
	Finish();
}
