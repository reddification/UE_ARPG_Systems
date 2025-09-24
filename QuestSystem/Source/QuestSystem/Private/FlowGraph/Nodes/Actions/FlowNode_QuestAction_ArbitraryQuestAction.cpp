// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_ArbitraryQuestAction.h"

#include "Data/QuestTypes.h"
#include "Objects/ArbitraryQuestAction.h"

EQuestActionExecuteResult UFlowNode_QuestAction_ArbitraryQuestAction::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	if (ensure(!ArbitraryQuestActionClass.IsNull()))
	{
		auto ActionClass = ArbitraryQuestActionClass.LoadSynchronous();
		auto ArbitraryQuestActionInstance = NewObject<UArbitraryQuestAction>(Context.Player.GetObject(), ActionClass);
		if (ensure(ArbitraryQuestActionInstance))
		{
			FArbitraryQuestActionContext ArbitraryQuestActionContext { Cast<APawn>(Context.Player.GetObject()) };
			ArbitraryQuestActionInstance->Execute(ArbitraryQuestActionContext, TagParameters, FloatParameters);
		}
	}

	return EQuestActionExecuteResult::Success;
}
