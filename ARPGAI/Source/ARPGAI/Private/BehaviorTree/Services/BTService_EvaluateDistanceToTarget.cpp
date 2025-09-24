// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_EvaluateDistanceToTarget.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Data/LogChannels.h"
#include "Data/NpcCombatTypes.h"

UBTService_EvaluateDistanceToTarget::UBTService_EvaluateDistanceToTarget()
{
	NodeName = "Evaluate distance to target";
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = true;
	OutDistanceBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateDistanceToTarget, OutDistanceBBKey));
	UpdateConditionBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateDistanceToTarget, UpdateConditionBBKey));
	UpdateConditionBBKey.AllowNoneAsValue(true);
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateDistanceToTarget, TargetBBKey), AActor::StaticClass());
	OutEvaluatedTargetMoveDirectionBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateDistanceToTarget, TargetBBKey),
		StaticEnum<ENpcTargetDistanceEvaluation>());
	OutCurrentDistanceBehaviorDurationBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateDistanceToTarget, OutCurrentDistanceBehaviorDurationBBKey.SelectedKeyName));
	OutCurrentDistanceBehaviorDurationBBKey.AllowNoneAsValue(true);
}

void UBTService_EvaluateDistanceToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto Blackboard = OwnerComp.GetBlackboardComponent();

#if WITH_EDITOR
	//ensure(!bUseUpdateCondition || Blackboard->GetValueAsBool(UpdateConditionBBKey.SelectedKeyName));
#endif

	auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
	if (Target == nullptr)
		return;

	ENpcTargetDistanceEvaluation PreviousDistanceEvaluation = static_cast<ENpcTargetDistanceEvaluation>(Blackboard->GetValueAsEnum(OutEvaluatedTargetMoveDirectionBBKey.SelectedKeyName));
	ENpcTargetDistanceEvaluation NewDistanceEvaluation = PreviousDistanceEvaluation;
	float CurrentDistanceBehaviorDuration = 0.f;
	if (!OutCurrentDistanceBehaviorDurationBBKey.IsNone())
	{
		CurrentDistanceBehaviorDuration = Blackboard->GetValueAsFloat(OutCurrentDistanceBehaviorDurationBBKey.SelectedKeyName);
		Blackboard->SetValueAsFloat(OutCurrentDistanceBehaviorDurationBBKey.SelectedKeyName, CurrentDistanceBehaviorDuration + DeltaSeconds);
	}
	
	auto Npc = OwnerComp.GetAIOwner()->GetPawn();
	const FVector NpcLocation = Npc->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	// TODO measure what time it takes to take a value from blackboard

	FBTMemory_EvaluateDistanceToTarget* BTMemory = reinterpret_cast<FBTMemory_EvaluateDistanceToTarget*>(NodeMemory);
	float OldDistance = 0.f;
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(Experiment_GetFloatFromBlackboard);	
		OldDistance = Blackboard->GetValueAsFloat(OutDistanceBBKey.SelectedKeyName);
	}

	// TODO perhaps change evaluation method to the following:
	// calculate delta distance for both NPC and target. Because now it can happen that target is backing away from NPC and NPC is following
	// it with relatively same speed. So in this case accumulated delta might be below threshold
	// might bypass by starting sprinting after 10s or something like that
	float NewDistance = OldDistance;
	if (bUsePathfinding)
	{
		auto NavSys = UNavigationSystemV1::GetCurrent(OwnerComp.GetAIOwner());
		// ENavigationQueryResult::Type Result = NavSys->GetPathLength(OwnerComp.GetAIOwner(), NpcLocation,
		// 	TargetLocation, NewDistance);

		auto Controller = OwnerComp.GetAIOwner();
		const ANavigationData* NavData = NavSys->GetNavDataForProps(Controller->GetNavAgentPropertiesRef(), NpcLocation);
		if (NavData)
		{
			FPathFindingQuery Query(Controller, *NavData, NpcLocation, TargetLocation);
			FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, EPathFindingMode::Regular);
			// FPathFindingResult Result2 = NavSys->FindPathSync(Query, EPathFindingMode::Hierarchical);
			// float PathLength2 = Result1.Path->GetLength();
			 NewDistance = PathFindingResult.Path->GetLength();

			if (PathFindingResult.Result == ENavigationQueryResult::Type::Fail)
				UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Warning, TEXT("BTService_EvaluateDistanceToTarget: Can't find path to target"));
		}
	}
	else
	{
		NewDistance = bUseSquareDistance ? (TargetLocation - NpcLocation).SizeSquared() : (TargetLocation - NpcLocation).Size();
	}

	// stupid fucking BTService Call tick on search start is called before OnBecomeRelevant where i'm caching this component
	if (!BTMemory->NpcCombatLogicComponent.IsValid())
		BTMemory->NpcCombatLogicComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcCombatLogicComponent>();

	BTMemory->NpcCombatLogicComponent->SetDistanceToTarget(NewDistance);
	
	if (OldDistance > 0.f)
	{
		UE_VLOG_CAPSULE(OwnerComp.GetAIOwner(), LogARPGAI, VeryVerbose, TargetLocation - FVector::UpVector * 90, 90, 30, FQuat::Identity, FColor::White,
			TEXT("Distance evaluators target"));
		
		float Delta = NewDistance - OldDistance;
		BTMemory->AccumulatedTargetDeltasCount++;
		float TargetDirectionAgainstNpcDotProduct = (NpcLocation - TargetLocation).GetSafeNormal() | (TargetLocation - BTMemory->PreviousTargetLocation).GetSafeNormal();
		float TargetMovedDistance = (TargetLocation - BTMemory->PreviousTargetLocation).Size() * -TargetDirectionAgainstNpcDotProduct;
		BTMemory->AccumulatedTargetDeltaDistance += TargetMovedDistance;
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, VeryVerbose, TEXT("TargetMovedDistance = %.2f; AccumulatedTargetDeltaDistance = %.2f"), TargetMovedDistance,
			BTMemory->AccumulatedTargetDeltaDistance);
		
		if (BTMemory->AccumulatedTargetDeltasCount > AccumulatedTargetDeltasCapacity)
		{
			NewDistanceEvaluation = ENpcTargetDistanceEvaluation::TargetIsStationary;
			if (BTMemory->AccumulatedTargetDeltaDistance > NonStationaryAccumulatedTargetDeltaDistance)
			{
				NewDistanceEvaluation = ENpcTargetDistanceEvaluation::TargetIsGettingAway;
			}
			else if (BTMemory->AccumulatedTargetDeltaDistance < -NonStationaryAccumulatedTargetDeltaDistance)
			{
				NewDistanceEvaluation = ENpcTargetDistanceEvaluation::TargetIsApproaching;
			}

			if (PreviousDistanceEvaluation != NewDistanceEvaluation)
			{
				Blackboard->SetValueAsEnum(OutEvaluatedTargetMoveDirectionBBKey.SelectedKeyName, static_cast<uint8>(NewDistanceEvaluation));
				if (!OutCurrentDistanceBehaviorDurationBBKey.IsNone())
					Blackboard->SetValueAsFloat(OutCurrentDistanceBehaviorDurationBBKey.SelectedKeyName, 0.f);
			}
			
			BTMemory->AccumulatedTargetDeltaDistance = 0.f;
			BTMemory->AccumulatedTargetDeltasCount = 0;
		}
	}

	BTMemory->PreviousTargetLocation = TargetLocation;
	
	if (FMath::Abs(NewDistance - OldDistance) > UpdateDistanceThreshold)
		Blackboard->SetValueAsFloat(OutDistanceBBKey.SelectedKeyName, NewDistance);
}

void UBTService_EvaluateDistanceToTarget::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	FBTMemory_EvaluateDistanceToTarget* BTMemory = reinterpret_cast<FBTMemory_EvaluateDistanceToTarget*>(NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	if (bUseUpdateCondition)
	{
		bool CurrentUpdateState = Blackboard->GetValueAsBool(UpdateConditionBBKey.SelectedKeyName);
		BTMemory->DelegateHandle = Blackboard->RegisterObserver(UpdateConditionBBKey.GetSelectedKeyID(), this,
			FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_EvaluateDistanceToTarget::OnCheckConditionUpdated));
		if (CurrentUpdateState)
			ScheduleNextTick(OwnerComp, NodeMemory);
		else 
			SetNextTickTime(NodeMemory, FLT_MAX);
	}

	Blackboard->SetValueAsEnum(OutEvaluatedTargetMoveDirectionBBKey.SelectedKeyName, static_cast<uint8>(ENpcTargetDistanceEvaluation::TargetIsStationary));
	auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
	if (ensure(Target))
	{
		auto NpcPawn = OwnerComp.GetAIOwner()->GetPawn();;
		// 16.01.2025 @AK: TODO handle pathfinding case
		float Distance = bUseSquareDistance
			? (Target->GetActorLocation() - NpcPawn->GetActorLocation()).SizeSquared()
			: (Target->GetActorLocation() - NpcPawn->GetActorLocation()).Size();
		Blackboard->SetValueAsFloat(OutDistanceBBKey.SelectedKeyName, Distance);
	}

	BTMemory->NpcCombatLogicComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcCombatLogicComponent>();
}

void UBTService_EvaluateDistanceToTarget::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTMemory_EvaluateDistanceToTarget* BTMemory = reinterpret_cast<FBTMemory_EvaluateDistanceToTarget*>(NodeMemory);
	if (bUseUpdateCondition)
	{
		OwnerComp.GetBlackboardComponent()->UnregisterObserver(UpdateConditionBBKey.GetSelectedKeyID(), BTMemory->DelegateHandle);
	}

	BTMemory->NpcCombatLogicComponent->SetDistanceToTarget(FLT_MAX);
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_EvaluateDistanceToTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BBData = Asset.GetBlackboardAsset())
	{
		UpdateConditionBBKey.ResolveSelectedKey(*BBData);
		OutCurrentDistanceBehaviorDurationBBKey.ResolveSelectedKey(*BBData);
	}
}

EBlackboardNotificationResult UBTService_EvaluateDistanceToTarget::OnCheckConditionUpdated(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	ensure(Key == UpdateConditionBBKey.GetSelectedKeyID());
	ensure(bUseUpdateCondition);
	bool bNewUpdateState = BlackboardComponent.GetValue<UBlackboardKeyType_Bool>(Key);
	auto BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	auto NodeMemory = BehaviorTreeComponent->GetNodeMemory(this, BehaviorTreeComponent->FindInstanceContainingNode(this));
	
	if (bNewUpdateState)
		ScheduleNextTick(*BehaviorTreeComponent, NodeMemory);
	else 
		SetNextTickTime(NodeMemory, FLT_MAX);
	
	return EBlackboardNotificationResult::ContinueObserving;
}

FString UBTService_EvaluateDistanceToTarget::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT("[out] Distance BB: %s\n[out] Evaluated target move direction BB: %s\nTarget BB: %s\nConsidered updates buffer size: %d\nAccumulated distance to consider change in direction: %.2f"),
		*OutDistanceBBKey.SelectedKeyName.ToString(), *OutEvaluatedTargetMoveDirectionBBKey.SelectedKeyName.ToString(), *TargetBBKey.SelectedKeyName.ToString(),
		AccumulatedTargetDeltasCapacity, NonStationaryAccumulatedTargetDeltaDistance);

	Result = Result.Append(FString::Printf(TEXT("\n[out]Distance behavior duration BB: %s"), *OutCurrentDistanceBehaviorDurationBBKey.SelectedKeyName.ToString()));
	
	if (bUseUpdateCondition)
		Result = Result.Append(FString::Printf(TEXT("\nUpdate only when %s == true"), *UpdateConditionBBKey.SelectedKeyName.ToString()));
	
	if (bUseSquareDistance)
		Result = Result.Append(TEXT("\nConsider squared distances"));

	if (bUsePathfinding)
		Result = Result.Append(TEXT("\nUse pathfinding"));
	
	Result = Result.Append(FString::Printf(TEXT("\n%s"), *Super::GetStaticDescription()));
	return Result;
}
