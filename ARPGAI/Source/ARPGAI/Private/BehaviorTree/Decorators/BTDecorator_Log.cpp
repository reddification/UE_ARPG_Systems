


#include "BehaviorTree/Decorators/BTDecorator_Log.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTDecorator_Log::UBTDecorator_Log()
{
#if WITH_EDITORONLY_DATA
	NodeName = "Log";
	FlowAbortMode = EBTFlowAbortMode::Self; // TODO revert to self!!!
	bNotifyBecomeRelevant = 1;
	bNotifyCeaseRelevant = 1;
	// bNotifyActivation = 1;
	// bNotifyDeactivation = 1;
#endif
}

#if WITH_EDITORONLY_DATA

void UBTDecorator_Log::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	if (bOnActivate && IsValid(OwnerComp.GetAIOwner()))
	{
		ShowMessage(OwnerComp, ActivationMessage, OnActivateMessageScreenColor);
	}
}

void UBTDecorator_Log::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (bOnDeactivate)
	{
		ShowMessage(OwnerComp, DeactivationMessage, OnDeactivateMessageScreenColor);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTDecorator_Log::ShowMessage(const UBehaviorTreeComponent& OwnerComp, const FString& Message, const FColor& Color) const
{
	if (IsValid(OwnerComp.GetAIOwner()) == false)
	{
		return;	
	}

	if (const APawn* MobCharacter = OwnerComp.GetAIOwner()->GetPawn())
	{
		FString FullMessage = FString::Printf(TEXT("[%s] %.2fs:\n\t%s"), *MobCharacter->GetName(), GetWorld()->GetTimeSeconds(), *Message);
		if (LogParameters.Num() > 0)
		{
			const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
			FullMessage = FullMessage.Append(TEXT("\n\tParameters:"));
			for (const auto& LogParameter : LogParameters)
			{
				FString ParameterStringValue = GetParameterStringValue(BlackboardComponent, LogParameter.Value);
				FullMessage = FullMessage.Append(FString::Printf(TEXT("\n\t\t%s = %s"), *LogParameter.Key, *ParameterStringValue));
			}	
		}
		
		UE_LOG(LogBehaviorTree, Log, TEXT("%s"), *FullMessage)
		if (bAddOnScreenMessage)
		{
			GEngine->AddOnScreenDebugMessage(-1, TimeToDisplayMessage, Color, Message);
		}
	}
}

FString UBTDecorator_Log::GetParameterStringValue(const UBlackboardComponent* BlackboardComponent,
	const FBlackboardKeySelector& BBKey) const
{
	if (BBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		if (UObject* ObjectValue = BlackboardComponent->GetValueAsObject(BBKey.SelectedKeyName))
		{
			return ObjectValue->GetName();
		}
	}
	else if (BBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		return BlackboardComponent->GetValueAsVector(BBKey.SelectedKeyName).ToString();
	}
	else if (BBKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
	{
		return FString::Printf(TEXT("%.2f"), BlackboardComponent->GetValueAsFloat(BBKey.SelectedKeyName));
	}
	else if (BBKey.SelectedKeyType == UBlackboardKeyType_Bool::StaticClass())
	{
		return BlackboardComponent->GetValueAsBool(BBKey.SelectedKeyName) ? "true" : "false";
	}

	return "";
}

FString UBTDecorator_Log::GetStaticDescription() const
{
	FString Result = "";
	if (bOnActivate)
	{
		Result = Result.Append(FString::Printf(TEXT("On activate: %s\n"), *ActivationMessage));	
	}

	if (bOnDeactivate)
	{
		Result = Result.Append(FString::Printf(TEXT("On deactivate: %s\n"), *DeactivationMessage));	
	}

	if (bAddOnScreenMessage)
	{
		Result = Result.Append("Also add on screen");
	}

	if (LogParameters.Num() > 0 && bShowParamsInDescription)
	{
		Result = Result.Append("\nParameters:");
		for (const auto& LogParameter : LogParameters)
		{
			Result = Result.Append(FString::Printf(TEXT("\n%s = %s"), *LogParameter.Key, *LogParameter.Value.SelectedKeyName.ToString()));
		}
	}
	
	return Result;
}

void UBTDecorator_Log::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = Asset.GetBlackboardAsset())
	{
		for (auto& LogParameter : LogParameters)
		{
			LogParameter.Value.ResolveSelectedKey(*BBAsset);
		}
	}
}

void UBTDecorator_Log::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (bOnActivate_Debug && IsValid(SearchData.OwnerComp.GetAIOwner()))
	{
		ShowMessage(SearchData.OwnerComp, ActivationMessage, OnActivateMessageScreenColor);
	}
}

void UBTDecorator_Log::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	if (bOnDeactivate_Debug)
	{
		ShowMessage(SearchData.OwnerComp, DeactivationMessage, OnDeactivateMessageScreenColor);
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

bool UBTDecorator_Log::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return true;
}

#endif
