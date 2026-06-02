// Fill out your copyright notice in the Description page of Project Settings.

#include "BehaviorTree/Decorators/BTDecorator_GameplayTagCondition.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_GameplayTagCondition::UBTDecorator_GameplayTagCondition()
{
	NodeName = "Check gameplay tags";
	BlackboardKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_GameplayTagCondition, BlackboardKey)));
}

bool UBTDecorator_GameplayTagCondition::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	auto Tags = OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_GameplayTag>(BlackboardKey.GetSelectedKeyID());
	if (bUseTagQuery)
	{
		if (GameplayTagQuery.IsEmpty())
			return Tags.IsEmpty();
		
		return GameplayTagQuery.Matches(Tags);
	}
	else
	{
		if (GameplayTagContainer.IsEmpty())
			return Tags.IsEmpty();
		
		return bContainerMatchAll ? Tags.HasAll(GameplayTagContainer) : Tags.HasAny(GameplayTagContainer);
	}
}

FString UBTDecorator_GameplayTagCondition::GetStaticDescription() const
{
	FString Result;
	if (bUseTagQuery)
	{
		Result = FString::Printf(TEXT("Check %s matches query"), *BlackboardKey.SelectedKeyName.ToString());
	}
	else
	{
		if (GameplayTagContainer.IsEmpty())
		{
			Result = FString::Printf(TEXT("Check %s is empty"), *BlackboardKey.SelectedKeyName.ToString());
		}
		else
		{
			Result = FString::Printf(TEXT("Check %s has %s tags:"), *BlackboardKey.SelectedKeyName.ToString(), bContainerMatchAll ? TEXT("all") : TEXT("any"));
			FString TagsList;
			for (const auto& Tag : GameplayTagContainer)
				TagsList += TEXT("\n") + Tag.ToString();
			
			Result += TagsList;
		}
	}

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
