// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/BTTask_SetWeaponReady.h"

#include "AIController.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_SetWeaponReady::UBTTask_SetWeaponReady()
{
	NodeName = "Set weapon ready";
}

EBTNodeResult::Type UBTTask_SetWeaponReady::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto NPC = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
	if (ensure(NPC))
	{
		bool bAlreadyInRequestedState = NPC->RequestWeaponReady(bSetReady);
		if (!bAlreadyInRequestedState && bAwaitCompletion)
		{
			Super::ExecuteTask(OwnerComp, NodeMemory);
			return EBTNodeResult::InProgress;
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_SetWeaponReady::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto NPC = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
		NPC->CancelWeaponReady(bSetReady);
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_SetWeaponReady::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_SetWeaponReady_Completed;
}

FString UBTTask_SetWeaponReady::GetStaticDescription() const
{
	FString Result = bSetReady ? TEXT("Unsheathe weapon") : TEXT("Sheathe weapon");
	
	if (bAwaitCompletion)
		Result = Result.Append(FString::Printf(TEXT("\nWait for completion")));

	return Result;
}
