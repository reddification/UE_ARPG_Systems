// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/BTTask_Dodge.h"

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
	if (auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
		Npc->CancelDodge();
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_Dodge::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Dodge_Completed;
}
