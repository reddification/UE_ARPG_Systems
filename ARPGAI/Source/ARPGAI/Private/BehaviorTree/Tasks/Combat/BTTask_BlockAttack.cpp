// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Tasks/Combat/BTTask_BlockAttack.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcInterfaceComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcCombatTypes.h"
#include "Interfaces/NpcCombatInterface.h"

UBTTask_BlockAttack::UBTTask_BlockAttack()
{
	NodeName = "Block attack";
	BlockResultBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_BlockAttack, BlockResultBBKey), StaticEnum<ENpcBlockResult>());
}

EBTNodeResult::Type UBTTask_BlockAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	auto NpcCombatLogicComponent = GetNpcCombatLogicComponent(OwnerComp);
	if (!NpcCombatLogicComponent->IsReactingToIncomingAttack())
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("UBTTask_BlockAttack: NPC not reacting to any incoming attack. Early finish"));
		return EBTNodeResult::Failed;
	}
	
	auto OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (auto Npc = Cast<INpcCombatInterface>(OwnerPawn))
	{
		bool bBlocking = Npc->BlockAttack();
		if (bBlocking)
		{
			WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Block_ParriedAttack.GetTag().GetTagName());
			WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Block_BlockedAttack.GetTag().GetTagName());
			WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Block_Completed.GetTag().GetTagName());
			
			// why such formula - because i just felt that way
			if (auto NpcInterfaceComponent = OwnerPawn->FindComponentByClass<UNpcInterfaceComponent>())
			{
				float BackdashProbability = BackdashChanceCoefficient 
					* NpcCombatLogicComponent->GetReaction() * NpcCombatLogicComponent->GetIntelligence() 
					* NpcCombatLogicComponent->GetNormalizedStamina() * (1.2f - NpcCombatLogicComponent->GetAggression());
				
				UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Unclamped backdash probability on block = %.2f"), BackdashProbability);
				BackdashProbability = FMath::RandRange(0.01f, 0.92f);
				UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Flipping coin for backdash on blocking. Probability = %.2f"), BackdashProbability);
				if (FMath::RandRange(0.f, 1.f) <= BackdashProbability)
				{
					UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Backdashing on block"));
					NpcInterfaceComponent->Backdash();
				}
			}
		}
		
		return bBlocking ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_BlockAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Npc = Cast<INpcCombatInterface>(OwnerComp.GetAIOwner()->GetPawn()))
		Npc->CancelBlock();

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_BlockAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Block_Completed;
}

void UBTTask_BlockAttack::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID,
                                    bool bSuccess)
{
	if (Message == AIGameplayTags::AI_BrainMessage_Block_ParriedAttack.GetTag().GetTagName())
		HandleBlockResult(OwnerComp, ENpcBlockResult::Parried);
	else if (Message == AIGameplayTags::AI_BrainMessage_Block_BlockedAttack.GetTag().GetTagName())
		HandleBlockResult(OwnerComp, ENpcBlockResult::Blocked);
	else
		Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);
}

void UBTTask_BlockAttack::HandleBlockResult(UBehaviorTreeComponent& OwnerComp, ENpcBlockResult BlockResult)
{
	auto NpcCombatLogicComponent = GetNpcCombatLogicComponent(OwnerComp);
	if (NpcCombatLogicComponent->ShouldRetaliateAfterSuccessfulBlock(BlockResult))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsEnum(BlockResultBBKey.SelectedKeyName, static_cast<uint8>(BlockResult));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		if (auto Npc = Cast<INpcCombatInterface>(OwnerComp.GetAIOwner()->GetPawn()))
			Npc->CancelBlock();
	}
}

FString UBTTask_BlockAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out]Block result BB: %s\nBackdash chance modifier = %.2f\n%s"), *BlockResultBBKey.SelectedKeyName.ToString(), BackdashChanceCoefficient, *Super::GetStaticDescription());
}
