// 


#include "BehaviorTree/Tasks/BehaviorEvaluators/BTTask_SendBehaviorEvaluatorMessage.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent2.h"

UBTTask_SendBehaviorEvaluatorMessage::UBTTask_SendBehaviorEvaluatorMessage()
{
	NodeName = "Send behavior evaluator message";
}

EBTNodeResult::Type UBTTask_SendBehaviorEvaluatorMessage::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (!Message.IsValid())
		return EBTNodeResult::Failed;
	
	auto BEC = GetNpcBehaviorEvaluatorComponent_v2(OwnerComp);
	if (EvaluatorTag.IsValid())
		BEC->HandleMessage(EvaluatorTag, Message);
	else 
		BEC->BroadcastMessage(Message);
	
	return EBTNodeResult::Succeeded;
}

FString UBTTask_SendBehaviorEvaluatorMessage::GetStaticDescription() const
{
	return FString::Printf(TEXT("Send %s evaluator message:\n%s"), 
		(EvaluatorTag.IsValid() ? *EvaluatorTag.ToString() : TEXT("all")), *Message.ToString());
}
