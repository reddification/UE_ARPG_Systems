// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_ShowScreenText.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestSystemGameMode.h"

EQuestActionExecuteResult UFlowNode_QuestAction_ShowScreenText::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	Context.GameMode->ShowScreenText(Title, SubTitle, ScreenTypeTag, ShowDuration);
	return EQuestActionExecuteResult::Success;
}
