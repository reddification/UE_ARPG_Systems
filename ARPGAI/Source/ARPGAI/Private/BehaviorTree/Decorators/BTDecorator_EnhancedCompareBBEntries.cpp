


#include "BehaviorTree/Decorators/BTDecorator_EnhancedCompareBBEntries.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Name.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_String.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTDecorator_EnhancedCompareBBEntries::UBTDecorator_EnhancedCompareBBEntries()
{
	NodeName = "Compare BB Entries (enhanced)";
}

bool UBTDecorator_EnhancedCompareBBEntries::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	if (BlackboardKeyA.SelectedKeyType != BlackboardKeyB.SelectedKeyType)
	{
		return false;
	}

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardKeyA.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
	{
		float A = BlackboardComponent->GetValueAsFloat(BlackboardKeyA.SelectedKeyName);
		float B = BlackboardComponent->GetValueAsFloat(BlackboardKeyB.SelectedKeyName);
		return Compare(A, B);
	}
	else if (BlackboardKeyA.SelectedKeyType == UBlackboardKeyType_Int::StaticClass())
	{
		int A = BlackboardComponent->GetValueAsInt(BlackboardKeyA.SelectedKeyName);
		int B = BlackboardComponent->GetValueAsInt(BlackboardKeyB.SelectedKeyName);
		return Compare(A, B);
	}
	// TODO FNames are compared via FName::Compare
	else if (BlackboardKeyA.SelectedKeyType == UBlackboardKeyType_Name::StaticClass())
	{
		const FName& A = BlackboardComponent->GetValueAsName(BlackboardKeyA.SelectedKeyName);
		const FName& B = BlackboardComponent->GetValueAsName(BlackboardKeyB.SelectedKeyName);
		return Compare(A.ToString(), B.ToString());
	}
	else if (BlackboardKeyA.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		UObject* A = BlackboardComponent->GetValueAsObject(BlackboardKeyA.SelectedKeyName);
		UObject* B = BlackboardComponent->GetValueAsObject(BlackboardKeyB.SelectedKeyName);
		return Compare(A, B);
	}
	else if (BlackboardKeyA.SelectedKeyType == UBlackboardKeyType_String::StaticClass())
	{
		const FString& A = BlackboardComponent->GetValueAsString(BlackboardKeyA.SelectedKeyName);
		const FString& B = BlackboardComponent->GetValueAsString(BlackboardKeyB.SelectedKeyName);
		return Compare(A, B);
	}
	else if (BlackboardKeyA.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		const FVector& A = BlackboardComponent->GetValueAsVector(BlackboardKeyA.SelectedKeyName);
		const FVector& B = BlackboardComponent->GetValueAsVector(BlackboardKeyB.SelectedKeyName);
		return Compare(A.SizeSquared(), B.SizeSquared());
	}

	return false;
}

FString UBTDecorator_EnhancedCompareBBEntries::GetStaticDescription() const
{
	// return FString::Printf(TEXT("Is %s %s %s"), *BlackboardKeyA.SelectedKeyName.ToString(),
	// 	*UEnum::GetDisplayValueAsText<EEnhancedBlackBoardEntryComparison>(EnhancedBlackBoardEntryComparison).ToString(),
	// 	*BlackboardKeyB.SelectedKeyName.ToString());
	return Super::GetStaticDescription();
}
