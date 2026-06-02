#include "BehaviorTree/Tasks/Blackboard/BTTask_SetRandomIntegerInRange.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetRandomIntegerInRange::UBTTask_SetRandomIntegerInRange()
{
	NodeName = "Set random integer";
	OutIntBBKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SetRandomIntegerInRange, OutIntBBKey));
}

EBTNodeResult::Type UBTTask_SetRandomIntegerInRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	OwnerComp.GetBlackboardComponent()->SetValueAsInt(OutIntBBKey.SelectedKeyName, FMath::RandRange(Interval.Min, Interval.Max));
	return EBTNodeResult::Succeeded;
}

FString UBTTask_SetRandomIntegerInRange::GetStaticDescription() const
{
	return FString::Printf(TEXT("Set %s = Random [%d; %d]"), 
		*OutIntBBKey.SelectedKeyName.ToString(), Interval.Min, Interval.Max);
}
