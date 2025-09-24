// 


#include "BehaviorTree/Tasks/BTTask_ChangeFloatValue.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ChangeFloatValue::UBTTask_ChangeFloatValue()
{
	NodeName = "Change float value";
	FloatValueBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ChangeFloatValue, FloatValueBBKey));
}

EBTNodeResult::Type UBTTask_ChangeFloatValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto CurrentValue = Blackboard->GetValueAsFloat(FloatValueBBKey.SelectedKeyName);
	Blackboard->SetValueAsFloat(FloatValueBBKey.SelectedKeyName, CurrentValue + DeltaValue);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_ChangeFloatValue::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s %c= %.2f"),
		*FloatValueBBKey.SelectedKeyName.ToString(), DeltaValue >= 0 ? '+' : '-', DeltaValue >= 0 ? DeltaValue : -DeltaValue);
}
