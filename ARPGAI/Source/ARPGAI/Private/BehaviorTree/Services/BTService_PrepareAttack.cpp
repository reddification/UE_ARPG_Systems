// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_PrepareAttack.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcCombatInterface.h"
#include "Interfaces/NpcThreat.h"
#include "Settings/NpcCombatSettings.h"

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
	
	NpcToTargetDotProductBBKey.AllowNoneAsValue(true);
	NpcToTargetDotProductBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PrepareAttack, NpcToTargetDotProductBBKey));
	TargetToNpcDotProductBBKey.AllowNoneAsValue(true);
	TargetToNpcDotProductBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PrepareAttack, TargetToNpcDotProductBBKey));
	
	bTickIntervals = true;
}

void UBTService_PrepareAttack::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	auto AIController = OwnerComp.GetAIOwner();
	auto Npc = Cast<INpcCombatInterface>(AIController->GetPawn());
	if (Npc == nullptr)
	{
		ensure(false);
		SetNextTickTime(NodeMemory, FLT_MAX);
		return;
	}
	
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetBBKey.SelectedKeyName));
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	bool bCurrentAttackRequestState = Blackboard->GetValueAsBool(OutRequestAttackBBKey.SelectedKeyName);
	if (Target == nullptr)
	{
		if (bCurrentAttackRequestState)
		{
			UE_VLOG(AIController, LogARPGAI_PrepareAttack, Verbose, TEXT("Target is nullptr, removing request for attack"));
			Blackboard->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, false);
		}
		
		return;
	}
	
	auto Threat = Cast<INpcThreat>(Target);
	if (!ensure(Threat))
	{
		if (bCurrentAttackRequestState)
		{
			UE_VLOG(AIController, LogARPGAI_PrepareAttack, Verbose, TEXT("Target %s has no IThreat interface. Removing request for attack"), *Target->GetName());
			Blackboard->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, false);
		}
		
		return;
	}
	
	const bool bAttackActive = Blackboard->GetValueAsBool(AttackActiveBBKey.SelectedKeyName);
	if (!bAttackActive)
	{
		if (!Npc->CanAttack())
		{
			UE_VLOG(AIController, LogARPGAI_PrepareAttack, VeryVerbose, TEXT("NPC can't attack"));
			if (bCurrentAttackRequestState)
				Blackboard->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, false);
			
			return;
		}
	}
	
	FBTMemory_PrepareAttack* BTMemory = reinterpret_cast<FBTMemory_PrepareAttack*>(NodeMemory);
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	bool bNpcToTargetDotProductPasses = true;
	if (!NpcToTargetDotProductBBKey.IsNone())
	{
		float DP = Blackboard->GetValueAsFloat(NpcToTargetDotProductBBKey.SelectedKeyName);
		bNpcToTargetDotProductPasses = DP >= AttackRelevantDotProductThreshold;
	}
	
	bool bEnemyIsTurnedAwayDP = false;
	if (!TargetToNpcDotProductBBKey.IsNone())
	{
		float DP = Blackboard->GetValueAsFloat(TargetToNpcDotProductBBKey.SelectedKeyName);
		bEnemyIsTurnedAwayDP = DP <= 0.5f;
	}
	
	// if not attacking and dot product is good or AI is stupid enough to start its attack when enemy is attacking
	const bool bCanConsiderAttack = bNpcToTargetDotProductPasses && 
		(bEnemyIsTurnedAwayDP || !Threat->IsAttacking_NpcThreat() || FMath::RandRange(0.f, 1.f) > BTMemory->Intelligence);
	
#if WITH_EDITOR
	UE_VLOG(AIController, LogARPGAI_PrepareAttack, VeryVerbose, TEXT("%s"), bCanConsiderAttack ? TEXT("Can consider attack") : TEXT("Cannot consider attack"));
	UE_VLOG(AIController, LogARPGAI_PrepareAttack, VeryVerbose, TEXT("%s"), bNpcToTargetDotProductPasses ? TEXT("NPC to target dot product passes") : TEXT("NPC is looking wrong way"));
	UE_VLOG(AIController, LogARPGAI_PrepareAttack, VeryVerbose, TEXT("%s"), bEnemyIsTurnedAwayDP ? TEXT("Target is turned away") : TEXT("Target can see NPC"));
	UE_VLOG(AIController, LogARPGAI_PrepareAttack, VeryVerbose, TEXT("%s"), Threat->IsAttacking_NpcThreat() ? TEXT("Target is attacking") : TEXT("Target is not attacking"));
#endif
	
	FVector NpcLocation = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	if (bCanConsiderAttack)
	{
		const float Dist = (NpcLocation - TargetLocation).Size();
		float DistDeviated = BTMemory->NpcCombatComponent->GetIntellectAffectedDistance(Dist);
		const bool bTargetInAttackRange = DistDeviated < BTMemory->AttackRange && DistDeviated > BTMemory->TooCloseDistance;

#if WITH_EDITOR
		UE_VLOG(AIController, LogARPGAI_PrepareAttack, VeryVerbose, TEXT("True attack range = %.2f\nPerceived Attack Range = %.2f\nTrue distance to target = %.2f\nPerceived distance to target = %.2f"), 
			Npc->GetAttackRange_NpcCombat(), BTMemory->AttackRange, Dist, DistDeviated);
		UE_VLOG(AIController, LogARPGAI_PrepareAttack, VeryVerbose, TEXT("Too close distance = %.2f\n"), BTMemory->TooCloseDistance);
		UE_VLOG(AIController, LogARPGAI_PrepareAttack, Verbose, TEXT("%s"), bTargetInAttackRange ? TEXT("TARGET IN RANGE") : TEXT("Target not in range"));
		UE_VLOG(AIController, LogARPGAI_PrepareAttack, Verbose, TEXT("Calculation: %.2f < %.2f && %.2f > %.2f"), DistDeviated, BTMemory->AttackRange, DistDeviated, BTMemory->TooCloseDistance);
		UE_VLOG_CAPSULE(OwnerComp.GetAIOwner(), LogARPGAI_PrepareAttack, VeryVerbose, Target->GetActorLocation() - FVector::UpVector * 90, 90, 30, FQuat::Identity, FColor::Red, TEXT("Target"));
#endif
	
		if (bTargetInAttackRange && !bCurrentAttackRequestState)
		{
			if (BTMemory->NpcCombatComponent->DecideWantToBaitAttack())
			{
				UE_VLOG(AIController, LogARPGAI_PrepareAttack, Verbose, TEXT("NPC decided to bait attack to try to parry"));
				SetNextTickTime(NodeMemory, BTMemory->NpcCombatComponent->GetBaitAttackDuration());
				return;
			}

			const bool bAttackWayBlocked = IsAnythingBlocksAttack(BTMemory, Target, Pawn, NpcLocation, TargetLocation);
			if (!bAttackWayBlocked)
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, true);
				SetNextTickTime(NodeMemory, NextTickDelayAfterRequestToAttack);
				UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_PrepareAttack, Verbose, TEXT("Must attack now. Setting flag"));
			}
			else
			{
				UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_PrepareAttack, Verbose, TEXT("Can't attack - something (or someone) is in the way"));
			}
		}
		else if (bCurrentAttackRequestState)
		{
			bool bTargetTooClose = DistDeviated < BTMemory->TooCloseDistance;
			bool bTargetTooFar = DistDeviated > BTMemory->AttackRange + ResetPreparedAttackDistanceThreshold; 
			if (bTargetTooFar || bTargetTooClose)
			{
				Blackboard->SetValueAsBool(OutRequestAttackBBKey.SelectedKeyName, false);
#if WITH_EDITOR
				if (bTargetTooFar)
				{
					UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_PrepareAttack, Verbose, TEXT("Cancelling attack request because target got too far (%.2f > %.2f + %.2f)"),
						DistDeviated, BTMemory->AttackRange, ResetPreparedAttackDistanceThreshold);
				}
				else if (bTargetTooClose)
				{
					UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_PrepareAttack, Verbose, TEXT("Cancelling attack request because target got too close (%.2f < %.2f)"),
						DistDeviated, BTMemory->TooCloseDistance);
				}
#endif
			}
		}
	}
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
	ServiceMemory->AttackTraceChannel = GetDefault<UNpcCombatSettings>()->AttackTraceChannel;
		
	FOnBlackboardChangeNotification OnAttackRangeChangeDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_PrepareAttack::OnAttackRangeChanged);
	FOnBlackboardChangeNotification OnTargetChangedDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_PrepareAttack::OnTargetChanged);
	
	Blackboard->RegisterObserver(AttackRangeBBKey.GetSelectedKeyID(), this, OnAttackRangeChangeDelegate);
	Blackboard->RegisterObserver(TargetBBKey.GetSelectedKeyID(), this, OnTargetChangedDelegate);
	
	ServiceMemory->TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
	if (ServiceMemory->TargetActor.IsValid())
		if (auto TargetThreat = Cast<INpcThreat>(ServiceMemory->TargetActor))
			TargetThreat->ReportPreparingAttack(OwnerComp.GetAIOwner()->GetPawn(), true);
	
	UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_PrepareAttack, Verbose, 
		TEXT("BT memory changed:\nAttack range = %2.f, too close distance = %.2f\nIntelligence = %.2f"),
		ServiceMemory->AttackRange, ServiceMemory->TooCloseDistance, ServiceMemory->Intelligence);
}

void UBTService_PrepareAttack::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
		Blackboard->UnregisterObserversFrom(this);
	
	if (FBTMemory_PrepareAttack* ServiceMemory = reinterpret_cast<FBTMemory_PrepareAttack*>(NodeMemory))
		if (ServiceMemory->TargetActor.IsValid())
			if (auto AIController = OwnerComp.GetAIOwner())
				if (auto Pawn = AIController->GetPawn())
					if (auto TargetThreat = Cast<INpcThreat>(ServiceMemory->TargetActor.Get()))
						TargetThreat->ReportPreparingAttack(Pawn, false);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTService_PrepareAttack::OnAttackRangeChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (BehaviorComp == nullptr)
		return EBlackboardNotificationResult::RemoveObserver;
	
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
	
	UE_VLOG(BehaviorComp->GetAIOwner(), LogARPGAI_PrepareAttack, Verbose, 
		TEXT("BT memory changed:\nAttack range = %2.f, too close distance = %.2f\nIntelligence = %.2f"),
		ServiceMemory->AttackRange, ServiceMemory->TooCloseDistance, ServiceMemory->Intelligence);
	
	return EBlackboardNotificationResult::ContinueObserving;
}

EBlackboardNotificationResult UBTService_PrepareAttack::OnTargetChanged(const UBlackboardComponent& BlackboardComponent,
	FBlackboard::FKey Key)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (BehaviorComp == nullptr || BehaviorComp->GetAIOwner() == nullptr || BehaviorComp->GetAIOwner()->GetPawn() == nullptr)
		return EBlackboardNotificationResult::RemoveObserver;

	auto Pawn = BehaviorComp->GetAIOwner()->GetPawn();
	UE_VLOG(Pawn, LogARPGAI_PrepareAttack, Log, TEXT("Target changed"));
	
	auto NodeMemory = BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(this));
	FBTMemory_PrepareAttack* ServiceMemory = reinterpret_cast<FBTMemory_PrepareAttack*>(NodeMemory);
	if (ServiceMemory != nullptr)
	{
		AActor* OldTarget = ServiceMemory->TargetActor.Get();
		UE_VLOG(Pawn, LogARPGAI_CombatLogic, Verbose, TEXT("Old target: %s"), OldTarget != nullptr ? *OldTarget->GetName() : TEXT("nullptr"));
		if (auto OldTargetThreat = Cast<INpcThreat>(OldTarget))
			OldTargetThreat->ReportPreparingAttack(Pawn, false);
		
		auto NewTarget = Cast<AActor>(BlackboardComponent.GetValueAsObject(TargetBBKey.SelectedKeyName));
		UE_VLOG(Pawn, LogARPGAI_CombatLogic, Verbose, TEXT("New target: %s"), NewTarget != nullptr ? *NewTarget->GetName() : TEXT("nullptr"));
		ServiceMemory->TargetActor = NewTarget;
		if (auto Threat = Cast<INpcThreat>(NewTarget))
			Threat->ReportPreparingAttack(Pawn, true);
	}
	else
	{
		UE_VLOG(Pawn, LogARPGAI_CombatLogic, Warning, TEXT("WTF no BT memory"));
		AActor* OldTarget = Cast<AActor>(BlackboardComponent.GetValueAsObject(TargetBBKey.SelectedKeyName));
		if (OldTarget)
			if (auto TargetThreat = Cast<INpcThreat>(OldTarget))
				TargetThreat->ReportPreparingAttack(Pawn, false);
		
		return EBlackboardNotificationResult::RemoveObserver;
	}

	if (NodeMemory != nullptr)
		SetNextTickTime(NodeMemory, 0.f);
	
	return EBlackboardNotificationResult::ContinueObserving;
}

bool UBTService_PrepareAttack::IsAnythingBlocksAttack(const FBTMemory_PrepareAttack* BTMemory, const AActor* Target, 
	const APawn* Pawn, const FVector& NpcLocation, const FVector& TargetLocation) const
{
	FHitResult SweepForwardResult;
	FVector Direction = (TargetLocation - NpcLocation).GetSafeNormal();
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(Pawn);
	CollisionQueryParams.AddIgnoredActor(Target);
	bool bAnythingBlocksAttack = GetWorld()->SweepSingleByChannel(SweepForwardResult, NpcLocation, TargetLocation - Direction * 30.f,
		FQuat::Identity, BTMemory->AttackTraceChannel, FCollisionShape::MakeCapsule(25, 60), CollisionQueryParams);
	
	if (bAnythingBlocksAttack)
	{
		if (auto TracedPawn = Cast<APawn>(SweepForwardResult.GetActor()))
		{
			const bool bAllyInTheWay = BTMemory->AttitudesComponent.IsValid() && BTMemory->AttitudesComponent->IsFriendly(TracedPawn);
			if (bAllyInTheWay && BTMemory->Intelligence <= 0.3f && FMath::RandRange(0.0f, 1.0f) <= 0.5f)
				bAnythingBlocksAttack = false;
		}	
		else
		{
			if (BTMemory->Intelligence <= 0.15f && FMath::RandRange(0.0f, 1.0f) <= 0.5f)
				bAnythingBlocksAttack = false;
		}
	}
	
	return bAnythingBlocksAttack;
}

void UBTService_PrepareAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BBAsset = Asset.GetBlackboardAsset())
	{
		AttackActiveBBKey.ResolveSelectedKey(*BBAsset);
		AttackRangeBBKey.ResolveSelectedKey(*BBAsset);
		NpcToTargetDotProductBBKey.ResolveSelectedKey(*BBAsset);
		TargetToNpcDotProductBBKey.ResolveSelectedKey(*BBAsset);
		TargetBBKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTService_PrepareAttack::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT("[out] Request Attack BB: %s\nTarget BB: %s\nIntelligence BB: %s\nAttack Range BB: %s\nReset prepared attack exit distance threshold = %.2f"),
		*OutRequestAttackBBKey.SelectedKeyName.ToString(), *TargetBBKey.SelectedKeyName.ToString(), *IntelligenceBBKey.SelectedKeyName.ToString(),
		*AttackRangeBBKey.SelectedKeyName.ToString(), ResetPreparedAttackDistanceThreshold);
	
	Result += FString::Printf(TEXT("\nStop attacking when dp [%s] < %.2f"), *NpcToTargetDotProductBBKey.SelectedKeyName.ToString(), AttackRelevantDotProductThreshold);
	Result += FString::Printf(TEXT("\nTarget to NPC DP BB: %s"), *TargetToNpcDotProductBBKey.SelectedKeyName.ToString());
	Result += FString::Printf(TEXT("\n%s"), *Super::GetStaticDescription());
	
	return Result;
}
