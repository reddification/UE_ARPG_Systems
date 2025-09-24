#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityBlackboard.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Composites/BTComposite_Utility.h"


UBTDecorator_UtilityBlackboard::UBTDecorator_UtilityBlackboard(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Blackboard Utility";

	UtilityValueKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_UtilityBlackboard, UtilityValueKey));
	UtilityValueKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_UtilityBlackboard, UtilityValueKey));
	FlowAbortMode = EBTFlowAbortMode::Self;
}

void UBTDecorator_UtilityBlackboard::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	UtilityValueKey.ResolveSelectedKey(*GetBlackboardAsset());
}

EBlackboardNotificationResult UBTDecorator_UtilityBlackboard::OnUtilityChanged_Obsolete(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTDecorator_UtilityBlackboard::OnUtilityChanged)
	
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	if (BehaviorComp == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	if (UtilityValueKey.GetSelectedKeyID() != ChangedKeyID)
		return EBlackboardNotificationResult::ContinueObserving;
	
	UBTComposite_Utility* UtilityComposite = Cast<UBTComposite_Utility>( GetParentNode());
	if (!ensure(UtilityComposite != nullptr))
	{
		BehaviorComp->RequestExecution(this);
		return EBlackboardNotificationResult::RemoveObserver;
	}

	uint8* BTUtilityMemoryPtr = BehaviorComp->GetNodeMemory(UtilityComposite, BehaviorComp->FindInstanceContainingNode(GetParentNode()));
	if (!ensure(BTUtilityMemoryPtr))
		return EBlackboardNotificationResult::RemoveObserver;
		
	FBTUtilityMemory* BTUtilityMemory = reinterpret_cast<FBTUtilityMemory*>(BTUtilityMemoryPtr);
	if (BTUtilityMemory->ActualUtilityNodesCount < 2)
		return EBlackboardNotificationResult::RemoveObserver;
	
	float NewUtilityValue = CalculateUtilityValue(*BehaviorComp, BTUtilityMemoryPtr);
	// const bool bUtilityBranchActive = BTUtilityMemory->ExecutionUtilityOrdering[0].ChildIdx == ChildIndex;
	// bool bNeedToUpdate = NewUtilityValue > BTUtilityMemory->ExecutionUtilityOrdering[0].UtilityScore && !bUtilityBranchActive
	// 	|| bUtilityBranchActive && NewUtilityValue < BTUtilityMemory->ExecutionUtilityOrdering[1].UtilityScore;

	// 29.11.2024 @AK: We don't need to call utility composite reevaluation if new utility value is not greater than current active branch
	// bool bNeedToUpdate = NewUtilityValue > BTUtilityMemory->ExecutionUtilityOrdering[0].UtilityScore + ;
	TArray<FIndexedUtilityValue> OldExecutionOrder(BTUtilityMemory->ExecutionUtilityOrdering, BTUtilityMemory->ActualUtilityNodesCount);
	for (auto i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
	{
		if (BTUtilityMemory->ExecutionUtilityOrdering[i].ChildIdx == ChildIndex)
		{
			BTUtilityMemory->ExecutionUtilityOrdering[i].UtilityScore = NewUtilityValue;
			break;
		}
	}

	Algo::StableSort(BTUtilityMemory->ExecutionUtilityOrdering);
	if (OldExecutionOrder[0].ChildIdx != BTUtilityMemory->ExecutionUtilityOrdering[0].ChildIdx)
	{
		BTUtilityMemory->bUtilityChanged = true;
		UtilityComposite->Reevaluate(BehaviorComp);
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

float UBTDecorator_UtilityBlackboard::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	float Value = 0.0f;

	if (UtilityValueKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
	{
		Value = MyBlackboard->GetValue<UBlackboardKeyType_Float>(UtilityValueKey.GetSelectedKeyID());
	}
	else if (UtilityValueKey.SelectedKeyType == UBlackboardKeyType_Int::StaticClass())
	{
		Value = (float)MyBlackboard->GetValue<UBlackboardKeyType_Int>(UtilityValueKey.GetSelectedKeyID());
	}

	return Value;
}

FString UBTDecorator_UtilityBlackboard::GetStaticDescription() const
{
	return FString::Printf(TEXT("Utility Key: %s"), *GetSelectedBlackboardKey().ToString());
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


