// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_PrepareAttack.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/Npc.h"
#include "Interfaces/Threat.h"

UBTService_PrepareAttack::UBTService_PrepareAttack()
{
	NodeName = "Prepare attack";
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	IntelligenceBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PrepareAttack, IntelligenceBBKey));
	AttackRangeBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PrepareAttack, AttackRangeBBKey));
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PrepareAttack, TargetBBKey), AActor::StaticClass());
	AttackActiveBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PrepareAttack, AttackActiveBBKey));
	OutRequestAttackBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PrepareAttack, OutRequestAttackBBKey));
	bTickIntervals = true;
}

void UBTService_PrepareAttack::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	FBTMemory_PrepareAttack* ServiceMemory = reinterpret_cast<FBTMemory_PrepareAttack*>(NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	ServiceMemory->Intelligence = Blackboard->GetValueAsFloat(IntelligenceBBKey.SelectedKeyName);
	float BaseAttackRange = Blackboard->GetValueAsFloat(AttackRangeBBKey.SelectedKeyName);
	
	auto NpcCombatComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcCombatLogicComponent>();
	ServiceMemory->NpcCombatComponent = NpcCombatComponent;
	
	ServiceMemory->AttackRange = NpcCombatComponent->GetIntellectAffectedDistance(BaseAttackRange);
	ServiceMemory->TooCloseDistance = ServiceMemory->AttackRange * TooCloseRangeDistanceCoefficient;
	ServiceMemory->AttackRange += LittleExtraAttackRange;

	FOnBlackboardChangeNotification OnBlackboardAttackRangeChangeNotification = FOnBlackboardChangeNotification::CreateUObject(this,
		&UBTService_PrepareAttack::OnAttackRangeChanged);

	ServiceMemory->OnAttackRangeChangedDelegateHandle = Blackboard->RegisterObserver(AttackRangeBBKey.GetSelectedKeyID(), this,
		OnBlackboardAttackRangeChangeNotification);

	if (auto TargetThreat = Cast<IThreat>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName)))
		TargetThreat->ReportPreparingAttack(OwnerComp.GetAIOwner()->GetPawn(), true);
}

void UBTService_PrepareAttack::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		FBTMemory_PrepareAttack* ServiceMemory = reinterpret_cast<FBTMemory_PrepareAttack*>(NodeMemory);
		Blackboard->UnregisterObserver(AttackRangeBBKey.GetSelectedKeyID(), ServiceMemory->OnAttackRangeChangedDelegateHandle);
		if (auto TargetThreat = Cast<IThreat>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName)))
			TargetThreat->ReportPreparingAttack(OwnerComp.GetAIOwner()->GetPawn(), false);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTService_PrepareAttack::OnAttackRangeChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (BehaviorComp == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}
	
	FBTMemory_PrepareAttack* ServiceMemory = reinterpret_cast<FBTMemory_PrepareAttack*>(BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(this)));
	if (ServiceMemory == nullptr) // can happen when combat BT is aborted (immediately after fight has finished) 
		return EBlackboardNotificationResult::RemoveObserver;
	
	auto Blackboard = BehaviorComp->GetBlackboardComponent();
	ServiceMemory->Intelligence = Blackboard->GetValueAsFloat(IntelligenceBBKey.SelectedKeyName);
	float BaseAttackRange = Blackboard->GetValueAsFloat(AttackRangeBBKey.SelectedKeyName);

	if (!BehaviorComp->GetAIOwner() || !BehaviorComp->GetAIOwner()->GetPawn())
		return EBlackboardNotificationResult::RemoveObserver;
	
	auto NpcCombatComponent = BehaviorComp->GetAIOwner()->GetPawn()->FindComponentByClass<UNpcCombatLogicComponent>();
	ServiceMemory->NpcCombatComponent = NpcCombatComponent;
	
	ServiceMemory->AttackRange = NpcCombatComponent->GetIntellectAffectedDistance(BaseAttackRange);
	ServiceMemory->TooCloseDistance = ServiceMemory->AttackRange * TooCloseRangeDistanceCoefficient; 
	ServiceMemory->AttackRange += LittleExtraAttackRange;
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTService_PrepareAttack::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetBBKey.SelectedKeyName));
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	if (Target == nullptr)
	{
		Blackboard->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, false);
		return;
	}
	
	auto Threat = Cast<IThreat>(Target);
	if (!ensure(Threat))
	{
		Blackboard->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, false);
		return;
	}

	const bool bAttackActive = Blackboard->GetValueAsBool(AttackActiveBBKey.SelectedKeyName);
	if (!bAttackActive)
	{
		auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
		if (!Npc->CanAttack())
		{
			Blackboard->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, false);
			if (bShowDebugInfo)
				GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Purple, TEXT("Can't attack"));
			
			return;
		}
	}
	
	FBTMemory_PrepareAttack* ServiceMemory = reinterpret_cast<FBTMemory_PrepareAttack*>(NodeMemory);
	// I don't really want to set BB value to false otherwise because conceptually this service must have a very low tick interval
	// and should be called in a branch where attack is not ready,
	// hence the branch that is executed when the attack IS ready should reset the OutRequestBBKey upon exit

	// if not attacking or AI is stupid enough to start its attack when enemy is attacking
	if (!Threat->IsAttacking() || FMath::RandRange(0.f, 1.f) > ServiceMemory->Intelligence)
	{
		const float Dist = (OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation() - Target->GetActorLocation()).Size();
		float DistDeviated = ServiceMemory->NpcCombatComponent->GetIntellectAffectedDistance(Dist);
		const bool bTargetInAttackRange = DistDeviated < ServiceMemory->AttackRange && DistDeviated > ServiceMemory->TooCloseDistance;
		
#if WITH_EDITORONLY_DATA
		if (bShowDebugInfo)
		{
			auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
			FString DebugMessage = FString::Printf(TEXT("True attack range = %.2f\nPerceived Attack Range = %.2f\n\nTrue distance to target = %.2f\nPerceived distance to target = %.2f"), 
				Npc->GetAttackRange(), ServiceMemory->AttackRange, Dist, DistDeviated);
			DebugMessage += FString::Printf(TEXT("\n\nToo close distance = %.2f\n"), ServiceMemory->TooCloseDistance);
			GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Orange, DebugMessage);
			if (bTargetInAttackRange)
				GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Emerald, TEXT("TARGET IN RANGE"));
			else 
				GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Red, TEXT("TARGET IS NOT IN RANGE"));
		}
#endif
		
		UE_VLOG_CAPSULE(OwnerComp.GetAIOwner(), LogARPGAI, VeryVerbose, Target->GetActorLocation() - FVector::UpVector * 90, 90, 30, FQuat::Identity, FColor::Red, TEXT("Target"));
		if (bTargetInAttackRange)
		{
			UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, VeryVerbose, TEXT("Must attack now"));
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, true);
			SetNextTickTime(NodeMemory, NextTickDelayAfterRequestToAttack);
		}
		else
		{
			bool bCurrentState = Blackboard->GetValueAsBool(OutRequestAttackBBKey.SelectedKeyName);
			if (bCurrentState)
			{
				if (DistDeviated > ServiceMemory->AttackRange + ResetPreparedAttackDistanceThreshold)
				{
					UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("Cancelling attack request"));
					Blackboard->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, false);
				}
			}
		}
	}
}

void UBTService_PrepareAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BBAsset = Asset.GetBlackboardAsset())
	{
		AttackActiveBBKey.ResolveSelectedKey(*BBAsset);
		AttackRangeBBKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTService_PrepareAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out] Request Attack BB: %s\nTarget BB: %s\nIntelligence BB: %s\nAttack Range BB: %s\nReset prepared attack exit distance threshold = %.2f\n%s"),
		*OutRequestAttackBBKey.SelectedKeyName.ToString(), *TargetBBKey.SelectedKeyName.ToString(), *IntelligenceBBKey.SelectedKeyName.ToString(),
		*AttackRangeBBKey.SelectedKeyName.ToString(), ResetPreparedAttackDistanceThreshold, *Super::GetStaticDescription());
}
