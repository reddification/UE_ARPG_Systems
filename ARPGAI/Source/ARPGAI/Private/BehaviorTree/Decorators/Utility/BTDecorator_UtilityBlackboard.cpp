#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityBlackboard.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"

UBTDecorator_UtilityBlackboard::UBTDecorator_UtilityBlackboard(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Blackboard Utility";
	
	UtilityValueKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_UtilityBlackboard, UtilityValueKey));
	FlowAbortMode = EBTFlowAbortMode::Both;
}

void UBTDecorator_UtilityBlackboard::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	UtilityValueKey.ResolveSelectedKey(*GetBlackboardAsset());
}

float UBTDecorator_UtilityBlackboard::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	float Value = 0.0f;

	if (ensure(UtilityValueKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass()))
		Value = MyBlackboard->GetValue<UBlackboardKeyType_Float>(UtilityValueKey.GetSelectedKeyID());

	return Value;
}

FString UBTDecorator_UtilityBlackboard::GetStaticDescription() const
{
	return FString::Printf(TEXT("Utility Key: %s\n%s"),
		*GetSelectedBlackboardKeyName().ToString(), bSuppressPrevious ? TEXT("Suppresses previous occurances") : TEXT(""));
}

void UBTDecorator_UtilityBlackboard::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);

	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	FString DescKeyValue;

	if (BlackboardComp)
	{
		DescKeyValue = BlackboardComp->DescribeKeyValue(UtilityValueKey.GetSelectedKeyID(), EBlackboardDescription::OnlyValue);
	}

	Values.Add(FString::Printf(TEXT("utility: %s"), *DescKeyValue));
}