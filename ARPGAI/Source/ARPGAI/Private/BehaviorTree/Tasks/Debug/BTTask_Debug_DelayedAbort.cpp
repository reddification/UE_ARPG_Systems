#include "BehaviorTree/Tasks/Debug/BTTask_Debug_DelayedAbort.h"

UBTTask_Debug_DelayedAbort::UBTTask_Debug_DelayedAbort()
{
	NodeName = "Delayed abort";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Debug_DelayedAbort::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UBTTask_Debug_DelayedAbort::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	if (OwnerComp.GetTaskStatus(this) == EBTTaskStatus::Aborting)
	{
		auto BTMemory = reinterpret_cast<FBTMemory_DelayedAbort*>(NodeMemory);
		BTMemory->RemainingAbortTime -= DeltaSeconds;
		if (BTMemory->RemainingAbortTime <= 0.f)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColorList::BlueViolet, TEXT("DELAY ENDED"));
			FinishLatentAbort(OwnerComp);
		}
	}
}

EBTNodeResult::Type UBTTask_Debug_DelayedAbort::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColorList::YellowGreen, TEXT("DELAY STARTED"));
	auto BTMemory = reinterpret_cast<FBTMemory_DelayedAbort*>(NodeMemory);
	BTMemory->RemainingAbortTime = AbortDelay;
	return EBTNodeResult::InProgress;
}
