// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/BTTask_GetParried.h"

#include "Data/AIGameplayTags.h"

UBTTask_GetParried::UBTTask_GetParried()
{
	NodeName = "Get parried";
}

void UBTTask_GetParried::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Attack_Parried;
}
