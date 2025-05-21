#include "BehaviorTree/Tasks/BTTask_Attack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "GameFramework/Character.h"
#include "Interfaces/Npc.h"
#include "Kismet/GameplayStatics.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
	OutAttackingBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, OutAttackingBBKey));
	OutTauntRequestBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, OutTauntRequestBBKey));
	RequestAttackBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, RequestAttackBBKey));
	OutAttackResultBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, OutAttackResultBBKey), StaticEnum<ENpcAttackResult>());
	OutDefenseActionBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, OutDefenseActionBBKey), StaticEnum<ENpcDefensiveAction>());
	DistanceToEnemyBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, DistanceToEnemyBBKey));
	AggressionBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, AggressionBBKey));
	
	bNotifyTick = true;
	bTickIntervals = true;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	auto TaskMemory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	if (TaskMemory->bPreparingNextAttack)
	{
		APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
		if (auto Npc = Cast<INpc>(Pawn))
		{
			UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("Morphing attack"));
			TaskMemory->bPreparingNextAttack = false;
			SetNextTickTime(NodeMemory, FLT_MAX);
			Npc->RequestNextAttack();
		}
	}
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto Npc = Cast<INpc>(Pawn);
	if (!ensure(Npc))
		return EBTNodeResult::Failed;

	WaitForMessage(OwnerComp, AttackHitTargetMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackWhiffedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackParriedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackCommitedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackStartedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackFinishedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackCancelledMessageTag.GetTagName());
	WaitForMessage(OwnerComp, EnemyBlockingAttackMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AbilityActivationFailedCantAffordMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AbilityActivationFailedConditionsNotMetMessageTag.GetTagName());

#if WITH_EDITOR
	AAIController* AIController = OwnerComp.GetAIOwner();
	UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("BTTask_Attack requesting attack"));
	UE_VLOG_CAPSULE(AIController, LogARPGAI, Verbose,
		UGameplayStatics::GetPlayerCharacter(this, 0)->GetActorLocation() - FVector::UpVector * 90,
		90, 30, FQuat::Identity, FColor::Orange, TEXT("Enemy"));
#endif

	SetNextTickTime(NodeMemory, FLT_MAX);
	Npc->StartAttack();
	
	// I'm not sure if I should duplicate this state to the blackboard. It might be needed for other nodes though
	auto AttackMemory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	// ensure(!AttackMemory->bAbortRequested);
	*AttackMemory = FBTMemory_Attack();
	// AttackMemory->bAttacking = true;
	// OwnerComp.GetBlackboardComponent()->SetValueAsBool(OutAttackingBBKey.SelectedKeyName, true);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID,
                               bool bSuccess)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("BTTask_Attack AIMessage: %s %s"), *Message.ToString(), bSuccess ? TEXT("success") : TEXT("failure"));
	auto AttackMemory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto Npc = Cast<INpc>(Pawn);
	auto NpcCombatComponent = Pawn->FindComponentByClass<UNpcCombatLogicComponent>();
	const bool bAttackStillRequested = Blackboard->GetValueAsBool(RequestAttackBBKey.SelectedKeyName);
	
	if (Message == AttackStartedMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::None;
		const auto& FeintParameters = NpcCombatComponent->GetAttackFeintParameters();
		const float FeintChance = FMath::RandRange(0.f, 1.f);
		if (FeintChance <= FeintParameters.Probability)
		{
			AttackMemory->bPreparingNextAttack = true;
			const float WorldTime = GetWorld()->GetTimeSeconds();
			const float FeintDelay = (Npc->GetAttackPhaseEndTime() - WorldTime) * FeintParameters.RelativeAttackWindUpDelay;
			SetNextTickTime(NodeMemory, FeintDelay);
			UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("Decided to feint in %2fs (chance = %.2f < %.2f)"), FeintDelay, FeintChance, FeintParameters.Probability);
		}
	}
	else if (Message == AttackCommitedMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::None;
		AttackMemory->bAttacking = true;
		Blackboard->SetValueAsBool(OutAttackingBBKey.SelectedKeyName, true);
	}
	else if (Message == AttackHitTargetMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::Hit;
		if (AttackMemory->bAbortRequested || !bAttackStillRequested)
		{
			FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, true);
			return;
		}

		float DistanceToEnemy = Blackboard->GetValueAsFloat(DistanceToEnemyBBKey.SelectedKeyName);
		if (TauntIfEnemyInRange.Contains(DistanceToEnemy))
		{
			if (NpcCombatComponent->GetTauntProbabilityOnSuccessfulAttack() <= FMath::RandRange(0.f, 1.f))
			{
				UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("Taunting opponent after successful attack"));
				Blackboard->SetValueAsEnum(OutAttackResultBBKey.SelectedKeyName, (uint8)ENpcAttackResult::Hit);
				Blackboard->SetValueAsBool(OutTauntRequestBBKey.SelectedKeyName, true);
				FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, true);
				return;
			}
		}

		AttackMemory->bPreparingNextAttack = true;
		SetNextTickTime(NodeMemory, FMath::RandRange(NextAttackDelay.GetLowerBound().GetValue(), NextAttackDelay.GetUpperBound().GetValue()));
		// Npc->RequestNextAttack();
	}
	else if (Message == AttackWhiffedMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::Whiffed;
		if (AttackMemory->bAbortRequested || !bAttackStillRequested)
		{
			FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, true);
			return;
		}
		
		if (NpcCombatComponent->GetBackstepProbabilityOnWhiff() < FMath::RandRange(0.f, 1.f))
		{
			UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("Decided to backstep after whiffed attack"));
			Blackboard->SetValueAsEnum(OutAttackResultBBKey.SelectedKeyName, (uint8)ENpcAttackResult::Whiffed);
			Blackboard->SetValueAsEnum(OutDefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::StepOut);
			FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, true);
		}
		else
		{
			UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("Requesting next attack"));
			// Npc->RequestNextAttack();
			AttackMemory->bPreparingNextAttack = true;
			SetNextTickTime(NodeMemory, FMath::RandRange(NextAttackDelay.GetLowerBound().GetValue(), NextAttackDelay.GetUpperBound().GetValue()));
		}
	}
	else if (Message == AttackFinishedMessageTag.GetTagName() || Message == AttackCancelledMessageTag.GetTagName())
	{
		FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, false);
	}
	else if (Message == AttackParriedMessageTag.GetTagName())
	{
		// this shouldn't even happen. what should happen is that the get parried/clash ability will be triggered and it will cancel attack ability
		// and attack ability's EndAbility will send AttackCanceledMessage
		ensure(false);
		AttackMemory->AttackResult = ENpcAttackResult::Parried;
		FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, false);
	}
	else if (Message == AbilityActivationFailedCantAffordMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::None;
		UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("Can't attack, can't afford cost. Resetting aggression because attack ability activation failed"));
		Blackboard->SetValueAsFloat(AggressionBBKey.SelectedKeyName, 0.f);
		FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, false);
	}
	else if (Message == AbilityActivationFailedConditionsNotMetMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::None;
		UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("Can't attack - GAS conditions not met"));
		OwnerComp.AddCooldownTagDuration(AttackActivationFailedCooldownTag, AttackActivationFailedCooldownTime, false);
		FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, false);
	}
	else if (Message == EnemyBlockingAttackMessageTag.GetTagName())
	{
		if (FMath::RandRange(0.f, 1.f) < AttackMemory->Intelligence * AttackMemory->Reaction)
		{
			UE_VLOG(AIController, LogARPGAI, Verbose, TEXT("BTTask_Attack: morphing one attack into another because target is blocking"));
			// morph attack
			Npc->RequestNextAttack();
		}
	}
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto AttackMemory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	if (AttackMemory->bAttacking)
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("BTTask_Attack requesting abort active attack"));
		AttackMemory->bAbortRequested = true;
		WaitForMessage(OwnerComp, AttackFinishedMessageTag.GetTagName());
		WaitForMessage(OwnerComp, AttackCancelledMessageTag.GetTagName());
		return EBTNodeResult::InProgress;
	}
	else
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("BTTask_Attack requesting abort, no active attack"));
		auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
		if (Npc)
			Npc->CancelAttack();
		
		return Super::AbortTask(OwnerComp, NodeMemory);
	}	
}

void UBTTask_Attack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	AttackParriedMessageTag = AIGameplayTags::AI_BrainMessage_Attack_Parried;
	AttackWhiffedMessageTag = AIGameplayTags::AI_BrainMessage_Attack_Whiffed;
	AttackHitTargetMessageTag = AIGameplayTags::AI_BrainMessage_Attack_HitTarget;
	AttackStartedMessageTag = AIGameplayTags::AI_BrainMessage_Attack_Started;
	AttackCommitedMessageTag = AIGameplayTags::AI_BrainMessage_Attack_Commited;
	AttackFinishedMessageTag = AIGameplayTags::AI_BrainMessage_Attack_Completed;
	AttackCancelledMessageTag = AIGameplayTags::AI_BrainMessage_Attack_Canceled;
	EnemyBlockingAttackMessageTag = AIGameplayTags::AI_BrainMessage_Attack_EnemyBlocking;
	AbilityActivationFailedCantAffordMessageTag = AIGameplayTags::AI_BrainMessage_Ability_ActivationFailed_CantAfford;
	AbilityActivationFailedConditionsNotMetMessageTag = AIGameplayTags::AI_BrainMessage_Ability_ActivationFailed_ConditionsNotMet;
}

void UBTTask_Attack::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	auto Memory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	Memory->bAttacking = false;
	Memory->bAbortRequested = false;
	Memory->bPreparingNextAttack = false;
}

void UBTTask_Attack::FinalizeAttack(UBehaviorTreeComponent& OwnerComp, FBTMemory_Attack* AttackMemory, UBlackboardComponent* Blackboard, INpc* Npc, bool bRequestFinishAttack) const
{
	Blackboard->SetValueAsBool(OutAttackingBBKey.SelectedKeyName, false);
	Blackboard->SetValueAsEnum(OutAttackResultBBKey.SelectedKeyName, static_cast<uint8>(AttackMemory->AttackResult));
	if (AttackMemory->bAbortRequested)
	{
		FinishLatentAbort(OwnerComp);
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("BTTask_Attack Finalizing - finish latent abort"));
	}
	else
	{
		FinishLatentTask(OwnerComp, AttackMemory->AttackResult == ENpcAttackResult::Hit ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("BTTask_Attack Finalizing - finish latent task"));
	}

	if (bRequestFinishAttack)
		Npc->CancelAttack();
}

uint16 UBTTask_Attack::GetInstanceMemorySize() const
{
	return sizeof(FBTMemory_Attack);
}

FString UBTTask_Attack::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out] Attacking BB: %s\n[out] Attack result BB: %s\n[out] Defense action BB: %s\n[out] Request taunt BB: %s\n[out] Aggression BB: %s\nAttack requested BB: %s\nTaunt if enemy in range[%.2f <= %s <= %.2f]"),
		*OutAttackingBBKey.SelectedKeyName.ToString(), *OutAttackResultBBKey.SelectedKeyName.ToString(),
		*OutDefenseActionBBKey.SelectedKeyName.ToString(), *OutTauntRequestBBKey.SelectedKeyName.ToString(),
		*AggressionBBKey.SelectedKeyName.ToString(), *RequestAttackBBKey.SelectedKeyName.ToString(),
		TauntIfEnemyInRange.GetLowerBound().GetValue(), *DistanceToEnemyBBKey.SelectedKeyName.ToString(), TauntIfEnemyInRange.GetUpperBound().GetValue());
}
