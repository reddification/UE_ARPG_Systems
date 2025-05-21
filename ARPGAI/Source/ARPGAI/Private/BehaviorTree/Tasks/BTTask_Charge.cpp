// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/BTTask_Charge.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_Charge::UBTTask_Charge()
{
	NodeName = "Charge";
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Charge, TargetBBKey), AActor::StaticClass());
	TargetBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Charge, TargetBBKey));
}

EBTNodeResult::Type UBTTask_Charge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto NPC = Cast<INpc>(Pawn);
	if (NPC == nullptr)
		return EBTNodeResult::Failed;
	
	FVector Direction = Pawn->GetActorForwardVector();
	if (bChargeToTarget)
	{
		auto Blackboard = OwnerComp.GetBlackboardComponent();
		if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			FVector Location = Blackboard->GetValueAsVector(TargetBBKey.SelectedKeyName);
			Direction = (Location - Pawn->GetActorLocation()).GetSafeNormal();
		}
		else if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
			if (ensure(Target))
			{
				Direction = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal();
			}
		}
	}
	
	NPC->ChargeIn(VerticalImpulseStrength, ForwardImpulseStrength, Direction);
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_Charge::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// it's better to wait for charge to complete, otherwise it can block some other abilities
	WaitForMessage(OwnerComp, CompletedMessageTag.GetTagName());
	return EBTNodeResult::InProgress;

	// if (auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
	// 	Npc->CancelChargeIn();
	//
	// return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_Charge::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BBAsset = Asset.GetBlackboardAsset())
	{
		TargetBBKey.ResolveSelectedKey(*BBAsset);
	}
	
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_ChargeIn_Completed;
}

FString UBTTask_Charge::GetStaticDescription() const
{
	return FString::Printf(TEXT("Vertical impulse: %.2f\nForward impulse: %.2f"), VerticalImpulseStrength, ForwardImpulseStrength);
}
