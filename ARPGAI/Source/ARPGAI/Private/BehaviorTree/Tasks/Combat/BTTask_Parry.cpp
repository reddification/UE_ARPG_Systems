// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/Combat/BTTask_Parry.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_Parry::UBTTask_Parry()
{
	NodeName = "Parry";
	ParriedAttackBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Parry, ParriedAttackBBKey));
}

EBTNodeResult::Type UBTTask_Parry::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	if (auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		bool bParrying = Npc->Parry();
		if (bParrying)
			WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Block_ParriedAttack.GetTag().GetTagName());
		
		return bParrying ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_Parry::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
		Npc->CancelParry();

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_Parry::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Block_Completed;
}

void UBTTask_Parry::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID,
	bool bSuccess)
{
	if (Message == AIGameplayTags::AI_BrainMessage_Block_ParriedAttack.GetTag().GetTagName())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(ParriedAttackBBKey.SelectedKeyName, true);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else
	{
		Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);
	}
}
