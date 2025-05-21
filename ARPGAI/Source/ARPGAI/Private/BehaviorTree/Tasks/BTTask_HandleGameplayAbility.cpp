﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/AIGameplayTags.h"

UBTTask_HandleGameplayAbility::UBTTask_HandleGameplayAbility()
{
	NodeName = "Handle gameplay ability";
}

EBTNodeResult::Type UBTTask_HandleGameplayAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	WaitForMessage(OwnerComp, CompletedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, ActivationFailedCantAffordTag.GetTagName());
	WaitForMessage(OwnerComp, ActivationFailedConditionsNotMetTag.GetTagName());
	return EBTNodeResult::InProgress;
}

void UBTTask_HandleGameplayAbility::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID,
	bool bSuccess)
{
	const EBTTaskStatus::Type Status = OwnerComp.GetTaskStatus(this);
	if (Status == EBTTaskStatus::Active)
	{
		if (Message == CompletedMessageTag.GetTagName())
		{
			FinishLatentTask(OwnerComp, bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
		}
		else if (Message == ActivationFailedCantAffordTag.GetTagName() || Message == ActivationFailedConditionsNotMetTag.GetTagName())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
		else
		{
			Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);
		}
	}
	else if (Status == EBTTaskStatus::Aborting)
	{
		FinishLatentAbort(OwnerComp);
	}
}

FString UBTTask_HandleGameplayAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Ability completed brain message: %s"), *CompletedMessageTag.ToString());
}

void UBTTask_HandleGameplayAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	ActivationFailedCantAffordTag = AIGameplayTags::AI_BrainMessage_Ability_ActivationFailed_CantAfford.GetTag();
	ActivationFailedConditionsNotMetTag = AIGameplayTags::AI_BrainMessage_Ability_ActivationFailed_ConditionsNotMet.GetTag();
}
