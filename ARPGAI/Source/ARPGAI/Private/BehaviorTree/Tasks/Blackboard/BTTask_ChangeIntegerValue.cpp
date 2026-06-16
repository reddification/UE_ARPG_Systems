#include "BehaviorTree/Tasks/Blackboard/BTTask_ChangeIntegerValue.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ChangeIntegerValue::UBTTask_ChangeIntegerValue()
{
	NodeName = "Change integer value";
	IntegerBBKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ChangeIntegerValue, IntegerBBKey));
}

EBTNodeResult::Type UBTTask_ChangeIntegerValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto CurrentValue = Blackboard->GetValueAsInt(IntegerBBKey.SelectedKeyName);
	Blackboard->SetValueAsInt(IntegerBBKey.SelectedKeyName, CurrentValue + DeltaValue);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_ChangeIntegerValue::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s %c= %d"),
		*IntegerBBKey.SelectedKeyName.ToString(), DeltaValue >= 0 ? '+' : '-', DeltaValue >= 0 ? DeltaValue : -DeltaValue);
}
