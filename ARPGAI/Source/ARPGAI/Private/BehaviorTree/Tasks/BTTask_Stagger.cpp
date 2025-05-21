// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/BTTask_Stagger.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/AIGameplayTags.h"

UBTTask_Stagger::UBTTask_Stagger()
{
	NodeName = "Stagger";
}

void UBTTask_Stagger::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Stagger_Completed;
}
