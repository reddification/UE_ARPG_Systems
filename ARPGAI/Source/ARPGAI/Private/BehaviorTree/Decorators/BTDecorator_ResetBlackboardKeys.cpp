// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_ResetBlackboardKeys.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_ResetBlackboardKeys::UBTDecorator_ResetBlackboardKeys()
{
	NodeName = "Reset blackboard keys";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_ResetBlackboardKeys::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (bOnActivation)
		ResetBlackboardKeys(SearchData.OwnerComp);
}

void UBTDecorator_ResetBlackboardKeys::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (bOnDeactivation)
		ResetBlackboardKeys(SearchData.OwnerComp);
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

void UBTDecorator_ResetBlackboardKeys::ResetBlackboardKeys(UBehaviorTreeComponent& BTComponent)
{
	auto Blackboard = BTComponent.GetBlackboardComponent();
	for (const auto& BBKey : BBKeys)
	{
		Blackboard->ClearValue(BBKey.SelectedKeyName);
	}
}

FString UBTDecorator_ResetBlackboardKeys::GetStaticDescription() const
{
	FString Result = "";
	if (bOnActivation)
		Result = Result.Append(TEXT("On activation\n"));

	if (bOnDeactivation)
		Result = Result.Append(TEXT("On deactivation\n"));

	Result = Result.Append(TEXT("Reset following BB keys:"));
	for (const auto& BBKey : BBKeys)
	{
		Result = Result.Append(FString::Printf(TEXT("\n%s"), *BBKey.SelectedKeyName.ToString()));	
	}

	return Result;
}