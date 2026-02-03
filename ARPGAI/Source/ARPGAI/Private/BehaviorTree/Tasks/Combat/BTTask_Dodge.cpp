// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/Combat/BTTask_Dodge.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_Dodge::UBTTask_Dodge()
{
	NodeName = "Dodge";
	DodgeLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Dodge, DodgeLocationBBKey));
}

EBTNodeResult::Type UBTTask_Dodge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
	auto DodgeLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(DodgeLocationBBKey.SelectedKeyName);
	bool bStarted = Npc->Dodge(DodgeLocation);
	return bStarted ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_Dodge::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// can't cancel dodge because of the nature of the motion
	// but have to re-register for brain message because BT component works in such a way that prior to calling AbortTask it unregisters all brain messages
	if (auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		if (Npc->IsDodgeActive())
		{
			WaitForMessage(OwnerComp, CompletedMessageTag.GetTagName());
			return EBTNodeResult::InProgress;
		}
	}
	
	return Super::AbortTask(OwnerComp, NodeMemory);
	// if (auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
	// 	Npc->CancelDodge();
}

void UBTTask_Dodge::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Dodge_Completed;
}
