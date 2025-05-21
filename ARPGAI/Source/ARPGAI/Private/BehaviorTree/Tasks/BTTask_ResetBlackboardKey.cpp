


#include "BehaviorTree/Tasks/BTTask_ResetBlackboardKey.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ResetBlackboardKey::UBTTask_ResetBlackboardKey()
{
	NodeName = "Reset blackboard key";
}

EBTNodeResult::Type UBTTask_ResetBlackboardKey::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	OwnerComp.GetBlackboardComponent()->ClearValue(BBKeyToReset.SelectedKeyName);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_ResetBlackboardKey::GetStaticDescription() const
{
	return FString::Printf(TEXT("Reset %s"), *BBKeyToReset.SelectedKeyName.ToString());
}
