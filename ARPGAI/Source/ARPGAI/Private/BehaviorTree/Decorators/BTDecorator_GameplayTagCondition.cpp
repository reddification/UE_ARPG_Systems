// Fill out your copyright notice in the Description page of Project Settings.

#include "BehaviorTree/Decorators/BTDecorator_GameplayTagCondition.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_GameplayTagCondition::UBTDecorator_GameplayTagCondition()
{
	NodeName = "Gameplay tags condition";
	BlackboardKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_GameplayTagCondition, BlackboardKey)));
}

bool UBTDecorator_GameplayTagCondition::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	auto Tags = OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_GameplayTag>(BlackboardKey.GetSelectedKeyID());
	return !Tags.IsEmpty()
		? bUseTagQuery
			? GameplayTagQuery.Matches(Tags)
			: GameplayTagContainer.IsEmpty()
				? Tags.IsEmpty()
				: Tags.HasAll(GameplayTagContainer)
		: false;
}

FString UBTDecorator_GameplayTagCondition::GetStaticDescription() const
{
	FString Result = bUseTagQuery
		? FString::Printf(TEXT("Check %s matches %s"),
			*BlackboardKey.SelectedKeyName.ToString(), *GameplayTagQuery.GetDescription())
		: !GameplayTagContainer.IsEmpty()
			? FString::Printf(TEXT("Check %s has tags %s"), *BlackboardKey.SelectedKeyName.ToString(), *GameplayTagContainer.ToStringSimple())
			: FString::Printf(TEXT("Check %s is empty"), *BlackboardKey.SelectedKeyName.ToString());

	Result += FString::Printf(TEXT("\n%s"), *Super::GetStaticDescription());

	return Result;
}

EBlackboardNotificationResult UBTDecorator_GameplayTagCondition::OnBlackboardKeyValueChange(
	const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	if (BehaviorComp == nullptr)
		return EBlackboardNotificationResult::RemoveObserver;

	if (BlackboardKey.GetSelectedKeyID() == ChangedKeyID)
		ConditionalFlowAbort(*BehaviorComp, EBTDecoratorAbortRequest::ConditionResultChanged);

	return EBlackboardNotificationResult::ContinueObserving;
}
