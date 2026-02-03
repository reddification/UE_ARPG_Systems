#include "BehaviorTree/Tasks/Combat/BTTask_AttackSequence.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "GameFramework/Character.h"
#include "Interfaces/Npc.h"
#include "Kismet/GameplayStatics.h"

UBTTask_AttackSequence::UBTTask_AttackSequence()
{
	NodeName = "Attack sequence";
	OutAttackingBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AttackSequence, OutAttackingBBKey));
	OutTauntRequestBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AttackSequence, OutTauntRequestBBKey));
	RequestAttackBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AttackSequence, RequestAttackBBKey));
	OutAttackResultBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AttackSequence, OutAttackResultBBKey), StaticEnum<ENpcAttackResult>());
	OutDefenseActionBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AttackSequence, OutDefenseActionBBKey), StaticEnum<ENpcDefensiveAction>());
	DistanceToEnemyBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AttackSequence, DistanceToEnemyBBKey));
	AggressionBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AttackSequence, AggressionBBKey));
	
	bNotifyTick = true;
	bTickIntervals = true;
}

void UBTTask_AttackSequence::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	auto TaskMemory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	if (TaskMemory->bAbortRequested)
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_Attack, Verbose, TEXT("UBTTask_AttackSequence::TickTask: pausing tick because abort is requested"));
		SetNextTickTime(NodeMemory, FLT_MAX);
		return;
	}
	
	if (TaskMemory->bPreparingNextAttack)
	{
		APawn* Pawn = OwnerComp.GetAIOwner()->GetPawn();
		if (auto Npc = Cast<INpc>(Pawn))
		{
			UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_Attack, Verbose, TEXT("Morphing attack"));
			TaskMemory->bPreparingNextAttack = false;
			SetNextTickTime(NodeMemory, FLT_MAX);
			Npc->RequestNextAttack();
		}
	}
}

EBTNodeResult::Type UBTTask_AttackSequence::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto Npc = Cast<INpc>(Pawn);
	if (!ensure(Npc))
		return EBTNodeResult::Failed;

	WaitForMessage(OwnerComp, AttackFinishedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackCancelledMessageTag.GetTagName());
	
	WaitForMessage(OwnerComp, AttackHitTargetMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackWhiffedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackParriedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackCommitedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AttackStartedMessageTag.GetTagName());
	WaitForMessage(OwnerComp, EnemyBlockingAttackMessageTag.GetTagName());
	
	WaitForMessage(OwnerComp, AbilityActivationFailedCantAffordMessageTag.GetTagName());
	WaitForMessage(OwnerComp, AbilityActivationFailedConditionsNotMetMessageTag.GetTagName());

#if WITH_EDITOR
	AAIController* AIController = OwnerComp.GetAIOwner();
	UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("Requesting attack"));
	if (auto FocusActor = AIController->GetFocusActor())
	{
		UE_VLOG_CAPSULE(AIController, LogARPGAI_Attack, Verbose, FocusActor->GetActorLocation() - FVector::UpVector * 90,
			90, 30, FQuat::Identity, FColor::Orange, TEXT("Enemy"));
	}
#endif

	SetNextTickTime(NodeMemory, FLT_MAX);
	Npc->StartAttack();
	
	// I'm not sure if I should duplicate this state to the blackboard. It might be needed for other nodes though
	auto AttackMemory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	// ensure(!AttackMemory->bAbortRequested);
	*AttackMemory = FBTMemory_Attack();
	auto CombatLogicComponent = GetNpcCombatLogicComponent(OwnerComp);
	AttackMemory->Intelligence = CombatLogicComponent->GetIntelligence();
	AttackMemory->Reaction = CombatLogicComponent->GetReaction();
	// AttackMemory->bAttacking = true;
	// OwnerComp.GetBlackboardComponent()->SetValueAsBool(OutAttackingBBKey.SelectedKeyName, true);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_AttackSequence::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID,
                               bool bSuccess)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("BTTask_Attack AIMessage: %s %s"), *Message.ToString(), bSuccess ? TEXT("success") : TEXT("failure"));
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
		if (FeintChance <= FeintParameters.Probability / AttackMemory->MorphCounters)
		{
			AttackMemory->MorphCounters++;
			AttackMemory->bPreparingNextAttack = true;
			const float WorldTime = GetWorld()->GetTimeSeconds();
			const float FeintDelay = (Npc->GetAttackPhaseEndTime() - WorldTime) * FeintParameters.RelativeAttackWindUpDelay;
			SetNextTickTime(NodeMemory, FeintDelay);
			UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("Decided to feint in %2fs (chance = %.2f < %.2f)"), FeintDelay, FeintChance, FeintParameters.Probability);
		}
	}
	else if (Message == AttackCommitedMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::None;
		AttackMemory->bAttacking = true;
		AttackMemory->MorphCounters = 1;
		Blackboard->SetValueAsBool(OutAttackingBBKey.SelectedKeyName, true);
#if WITH_EDITOR
		// WARNING! THIS PIECE OF CODE IS ONLY FOR DEBUGGING PURPOSES TO TEST A HYPOTHESIS 
		// if during an unabortable attack a dodge is requested and BT flow switch is denied because task doesn't allow to abort immediately 
		// AND then BEFORE attack is finished and hence task abort is also not happened yet A STAGGER is triggered
		// which calls PauseLogic on BT Component. and after stagger is finished and ResumeLogic is called on BTComponent, 
		// BTComponent doesn't do shit until ScheduleExecutionUpdate is called which should be called after finishing latent abort but it seems something in between all these events resets the flag
		// and hence BT hierarchy stops at inactive BTTask_AttackSequence even though the tree stack has pending RunBehavior "defense actions" tree activation
		// but it just doesn't happen until UBehaviorTreeComponent::ScheduleExecutionUpdate is called
		// so to test it and make sure this is the root of the problem, I will cause this bug to reproduce. 
		// REMOVE THIS CODE ASAP
		
		if (Debug_Options.Contains(1))
			GetNpcCombatLogicComponent(OwnerComp)->Debug_RequestDodge();
		if (Debug_Options.Contains(2))
			Blackboard->ClearValue(AggressionBBKey.SelectedKeyName);
		if (Debug_Options.Contains(3))
			GetNpcComponent(OwnerComp)->Debug_RequestStagger();
#endif
	}
	else if (Message == AttackHitTargetMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::Hit;
		if (AttackMemory->bAbortRequested || !bAttackStillRequested)
		{
			// FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, true);
			return;
		}

		float DistanceToEnemy = Blackboard->GetValueAsFloat(DistanceToEnemyBBKey.SelectedKeyName);
		if (TauntIfEnemyInRange.Contains(DistanceToEnemy))
		{
			if (FMath::RandRange(0.f, 1.f) <= NpcCombatComponent->GetTauntProbabilityOnSuccessfulAttack())
			{
				UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("Taunting opponent after successful attack"));
				Blackboard->SetValueAsEnum(OutAttackResultBBKey.SelectedKeyName, (uint8)ENpcAttackResult::Hit);
				Blackboard->SetValueAsBool(OutTauntRequestBBKey.SelectedKeyName, true);
				// FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, true);
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
			// FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, true);
			return;
		}
		
		if (FMath::RandRange(0.f, 1.f) <= NpcCombatComponent->GetBackstepProbabilityOnWhiff())
		{
			UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("Decided to backstep after whiffed attack"));
			Blackboard->SetValueAsEnum(OutAttackResultBBKey.SelectedKeyName, (uint8)ENpcAttackResult::Whiffed);
			Blackboard->SetValueAsEnum(OutDefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::Backdash);
			// FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, true);
		}
		else
		{
			UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("Requesting next attack"));
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
		// optionally BTComponent should go into pause
		ensure(false);
		AttackMemory->AttackResult = ENpcAttackResult::Parried;
		// FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, false);
	}
	else if (Message == AbilityActivationFailedCantAffordMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::None;
		UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("Can't attack, can't afford cost. Resetting aggression because attack ability activation failed"));
		Blackboard->SetValueAsFloat(AggressionBBKey.SelectedKeyName, 0.f);
		FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, false);
	}
	else if (Message == AbilityActivationFailedConditionsNotMetMessageTag.GetTagName())
	{
		AttackMemory->AttackResult = ENpcAttackResult::None;
		UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("Can't attack - GAS conditions not met"));
		OwnerComp.AddCooldownTagDuration(AttackActivationFailedCooldownTag, AttackActivationFailedCooldownTime, false);
		FinalizeAttack(OwnerComp, AttackMemory, Blackboard, Npc, false);
	}
	else if (Message == EnemyBlockingAttackMessageTag.GetTagName())
	{
		if (FMath::RandRange(0.f, 1.f) <= AttackMemory->Intelligence * AttackMemory->Reaction)
		{
			UE_VLOG(AIController, LogARPGAI_Attack, Verbose, TEXT("Morphing one attack into another because target is blocking"));
			// morph attack
			Npc->RequestNextAttack();
		}
	}
}

EBTNodeResult::Type UBTTask_AttackSequence::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto AttackMemory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	if (AttackMemory->bAttacking)
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_Attack, Verbose, TEXT("Requesting abort active attack"));
		AttackMemory->bAbortRequested = true;
		WaitForMessage(OwnerComp, AttackFinishedMessageTag.GetTagName());
		WaitForMessage(OwnerComp, AttackCancelledMessageTag.GetTagName());
		return EBTNodeResult::InProgress;
	}
	else
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_Attack, Verbose, TEXT("Requesting abort, no active attack"));
		auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
		if (Npc)
			Npc->CancelAttack();
		
		return Super::AbortTask(OwnerComp, NodeMemory);
	}	
}

void UBTTask_AttackSequence::InitializeFromAsset(UBehaviorTree& Asset)
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

void UBTTask_AttackSequence::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	auto Memory = reinterpret_cast<FBTMemory_Attack*>(NodeMemory);
	Memory->bAttacking = false;
	Memory->bAbortRequested = false;
	Memory->bPreparingNextAttack = false;
}

void UBTTask_AttackSequence::FinalizeAttack(UBehaviorTreeComponent& OwnerComp, FBTMemory_Attack* AttackMemory, UBlackboardComponent* Blackboard, INpc* Npc, bool bRequestFinishAttack) const
{
	Blackboard->SetValueAsBool(OutAttackingBBKey.SelectedKeyName, false);
	Blackboard->SetValueAsEnum(OutAttackResultBBKey.SelectedKeyName, static_cast<uint8>(AttackMemory->AttackResult));
	if (AttackMemory->bAbortRequested)
	{
		FinishLatentAbort(OwnerComp);
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_Attack, Verbose, TEXT("BTTask_Attack Finalizing - finish latent abort"));
	}
	else
	{
		FinishLatentTask(OwnerComp, AttackMemory->AttackResult == ENpcAttackResult::Hit ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_Attack, Verbose, TEXT("BTTask_Attack Finalizing - finish latent task"));
	}

	if (bRequestFinishAttack)
		Npc->CancelAttack();
}

uint16 UBTTask_AttackSequence::GetInstanceMemorySize() const
{
	return sizeof(FBTMemory_Attack);
}

FString UBTTask_AttackSequence::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out] Attacking BB: %s\n[out] Attack result BB: %s\n[out] Defense action BB: %s\n[out] Request taunt BB: %s\n[out] Aggression BB: %s\nAttack requested BB: %s\nTaunt if enemy in range[%.2f <= %s <= %.2f]"),
		*OutAttackingBBKey.SelectedKeyName.ToString(), *OutAttackResultBBKey.SelectedKeyName.ToString(),
		*OutDefenseActionBBKey.SelectedKeyName.ToString(), *OutTauntRequestBBKey.SelectedKeyName.ToString(),
		*AggressionBBKey.SelectedKeyName.ToString(), *RequestAttackBBKey.SelectedKeyName.ToString(),
		TauntIfEnemyInRange.GetLowerBound().GetValue(), *DistanceToEnemyBBKey.SelectedKeyName.ToString(), TauntIfEnemyInRange.GetUpperBound().GetValue());
}
