// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/Combat/BTTask_Charge.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
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
	
	FVector Location = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 350.f;
	if (bChargeToTarget)
	{
		auto Blackboard = OwnerComp.GetBlackboardComponent();
		if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			Location = Blackboard->GetValueAsVector(TargetBBKey.SelectedKeyName);
		}
		else if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
			if (ensure(Target))
				Location = Target->GetActorLocation();
		}
	}

	NPC->ChargeIn(VerticalImpulseStrength, ForwardImpulseStrength, Location);
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_Charge::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto AIController = OwnerComp.GetAIOwner())
	{
		if (auto Npc = Cast<INpc>(AIController->GetPawn()))
		{
			if (Npc->IsChargeInActive())
			{
				UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("UBTTask_Charge: abort was requested when task was active. Halted abort and re-registered for completed message"));
				WaitForMessage(OwnerComp, CompletedMessageTag.GetTagName());
				return EBTNodeResult::InProgress;
			}
		}
	}
	
	return Super::AbortTask(OwnerComp, NodeMemory);
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
	FString Description = FString::Printf(TEXT("Vertical impulse: %.2f\nForward impulse: %.2f"), VerticalImpulseStrength, ForwardImpulseStrength);
	if (bChargeToTarget)
		Description += FString::Printf(TEXT("\nCharge to %s"), *TargetBBKey.SelectedKeyName.ToString());
	
	return Description;
}
