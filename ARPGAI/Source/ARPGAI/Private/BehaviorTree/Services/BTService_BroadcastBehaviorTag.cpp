// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_BroadcastBehaviorTag.h"

#include "AIController.h"
#include "Components/NpcComponent.h"

UBTService_BroadcastBehaviorTag::UBTService_BroadcastBehaviorTag()
{
	NodeName = "Broadcast Behavior Tag";
	bNotifyBecomeRelevant = 1;
	bNotifyCeaseRelevant = 1;
	bNotifyTick = 0;
}

FString UBTService_BroadcastBehaviorTag::GetStaticDescription() const
{
	return BehaviorTag.ToString();
}

void UBTService_BroadcastBehaviorTag::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcComponent>()->OnBehaviorChanged.Broadcast(BehaviorTag, true);
}

void UBTService_BroadcastBehaviorTag::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcComponent>()->OnBehaviorChanged.Broadcast(BehaviorTag, false);
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}
