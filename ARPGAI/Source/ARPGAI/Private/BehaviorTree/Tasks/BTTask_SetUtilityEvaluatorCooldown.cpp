// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/BTTask_SetUtilityEvaluatorCooldown.h"

#include "AIController.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"

UBTTask_SetUtilityEvaluatorCooldown::UBTTask_SetUtilityEvaluatorCooldown()
{
	NodeName = "Set utility evaluator cooldown";
}

EBTNodeResult::Type UBTTask_SetUtilityEvaluatorCooldown::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (UtilityEvaluatorTags.IsEmpty())
		return EBTNodeResult::Succeeded;
	
	auto UtilityComponent = OwnerComp.GetAIOwner()->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();
	if (UtilityComponent == nullptr)
		return EBTNodeResult::Failed;

	for (const auto& Tag : UtilityEvaluatorTags)
		UtilityComponent->SetBehaviorEvaluatorCooldown(Tag, Cooldown);

	return EBTNodeResult::Succeeded;
}

FString UBTTask_SetUtilityEvaluatorCooldown::GetStaticDescription() const
{
	return FString::Printf(TEXT("Utility evaluators: %s\nCooldown = %.2f"), *UtilityEvaluatorTags.ToStringSimple(), Cooldown);
}
