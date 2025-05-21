


#include "BehaviorTree/Decorators/BTDecorator_SetBlackboardValue.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Name.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_String.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTDecorator_SetBlackboardValue::UBTDecorator_SetBlackboardValue()
{
	NodeName = "Set Blackboard Value (experimental)";
	bNotifyActivation = 1;
	bNotifyDeactivation = 1;
	FlowAbortMode = EBTFlowAbortMode::Type::Self;
}

void UBTDecorator_SetBlackboardValue::SetBlackboardValue(UBlackboardComponent* BlackboardComponent, FSetBlackboardValueMemory* NodeMemory, bool bApply)
{
	switch (BlackboardKeyDataType)
	{
	case EBlackboardKeyDataType::Float:
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
		{
			// BlackboardComponent->SetValueAsFloat(BlackboardKey.SelectedKeyName, FloatValue);
			SetValue<UBlackboardKeyType_Float>(BlackboardComponent, BlackboardKey.GetSelectedKeyID(), FloatValue,
			                                   NodeMemory->InitialFloatValue, NodeMemory->bWasSet, bApply);
			return;
		}
		break;
	case EBlackboardKeyDataType::Int:
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Int::StaticClass())
		{
			// BlackboardComponent->SetValueAsInt(BlackboardKey.SelectedKeyName, IntValue);
			SetValue<UBlackboardKeyType_Int>(BlackboardComponent, BlackboardKey.GetSelectedKeyID(), IntValue,
			                                 NodeMemory->InitialIntValue, NodeMemory->bWasSet, bApply);
			return;
		}
		break;
	case EBlackboardKeyDataType::Bool:
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Bool::StaticClass())
		{
			// BlackboardComponent->SetValueAsBool(BlackboardKey.SelectedKeyName, BoolValue);
			SetValue<UBlackboardKeyType_Bool>(BlackboardComponent, BlackboardKey.GetSelectedKeyID(), BoolValue,
			                                  NodeMemory->InitialBoolValue, NodeMemory->bWasSet, bApply, true);
			return;
		}
		
		break;
	case EBlackboardKeyDataType::GameplayTag:
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Name::StaticClass())
		{
			// BlackboardComponent->SetValueAsName(BlackboardKey.SelectedKeyName, TagValue.GetTagName());
			SetValue<UBlackboardKeyType_Name>(BlackboardComponent, BlackboardKey.GetSelectedKeyID(), TagValue.GetTagName(),
			                                  NodeMemory->InitialNameValue, NodeMemory->bWasSet, bApply);
			return;
		}
		
		break;
	case EBlackboardKeyDataType::String:
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_String::StaticClass())
		{
			// BlackboardComponent->SetValueAsString(BlackboardKey.SelectedKeyName, StringValue);
			SetValue<UBlackboardKeyType_String>(BlackboardComponent, BlackboardKey.GetSelectedKeyID(), StringValue,
			                                    NodeMemory->InitialStringValue, NodeMemory->bWasSet, bApply);
			return;
		}
	case EBlackboardKeyDataType::Vector:
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			// BlackboardComponent->SetValueAsVector(BlackboardKey.SelectedKeyName, VectorValue);
			SetValue<UBlackboardKeyType_Vector>(BlackboardComponent, BlackboardKey.GetSelectedKeyID(), VectorValue,
			                                    NodeMemory->InitialVectorValue, NodeMemory->bWasSet, bApply);
			return;
		}
	default:
		break;
	}

	ensure(false);
}

void UBTDecorator_SetBlackboardValue::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto NodeMemory = GetNodeMemory<FSetBlackboardValueMemory>(SearchData);
	SetBlackboardValue(SearchData.OwnerComp.GetBlackboardComponent(), NodeMemory, true);
}

void UBTDecorator_SetBlackboardValue::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	auto NodeMemory = GetNodeMemory<FSetBlackboardValueMemory>(SearchData);
	SetBlackboardValue(SearchData.OwnerComp.GetBlackboardComponent(), NodeMemory, false);
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_SetBlackboardValue::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT("%s = "), *BlackboardKey.SelectedKeyName.ToString());
	switch (BlackboardKeyDataType)
	{
	case EBlackboardKeyDataType::Float:
		return Result.Append(FString::Printf(TEXT("%.2f"), FloatValue));
	case EBlackboardKeyDataType::Int:
		return Result.Append(FString::Printf(TEXT("%d"), IntValue));
	case EBlackboardKeyDataType::Bool:
		return Result.Append(FString::Printf(TEXT("%s"), BoolValue ? TEXT("true") : TEXT("false")));
	case EBlackboardKeyDataType::GameplayTag:
		return Result.Append(FString::Printf(TEXT("%s"), *TagValue.ToString()));
	case EBlackboardKeyDataType::String:
		return Result.Append(FString::Printf(TEXT("%s"), *StringValue));
	case EBlackboardKeyDataType::Vector:
		return Result.Append(FString::Printf(TEXT("%s"), *VectorValue.ToString()));
	default:
		break;
	}

	return Result;
}

uint16 UBTDecorator_SetBlackboardValue::GetInstanceMemorySize() const
{
	return sizeof(FSetBlackboardValueMemory);
}

void UBTDecorator_SetBlackboardValue::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		BlackboardKey.ResolveSelectedKey(*BB);
	}
}

bool UBTDecorator_SetBlackboardValue::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
                                                                 uint8* NodeMemory) const
{
	return Super::CalculateRawConditionValue(OwnerComp, NodeMemory);
}
