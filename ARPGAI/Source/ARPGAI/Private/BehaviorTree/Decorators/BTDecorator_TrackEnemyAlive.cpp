// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_TrackEnemyAlive.h"

#include "AIController.h"
#include "Activities/ActivityInstancesHelper.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcComponent.h"

UBTDecorator_TrackEnemyAlive::UBTDecorator_TrackEnemyAlive()
{
	NodeName = "Track enemy alive";
	ActiveTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_TrackEnemyAlive, ActiveTargetBBKey), AActor::StaticClass());
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_TrackEnemyAlive::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);

	auto NpcCombatComponent = SearchData.OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcCombatLogicComponent>();
	if (!ensure(NpcCombatComponent))
		return;
	
	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	auto CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(ActiveTargetBBKey.SelectedKeyName));
	if (CurrentTarget != nullptr)
		NpcCombatComponent->TrackEnemyAlive(CurrentTarget);
	
	auto BlackboardObserver = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_TrackEnemyAlive::OnEnemyChanged);
	auto BTMemory = GetNodeMemory<FBTMemory_TrackEnemyAlive>(SearchData);
	BTMemory->BlackboardObserverDelegateHandle = Blackboard->RegisterObserver(ActiveTargetBBKey.GetSelectedKeyID(), this, BlackboardObserver);
}

void UBTDecorator_TrackEnemyAlive::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto AIController = SearchData.OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())
			if (auto NpcCombatComponent = Pawn->FindComponentByClass<UNpcCombatLogicComponent>())
				NpcCombatComponent->ResetTrackingEnemyAlive();

	if (auto BTMemory = GetNodeMemory<FBTMemory_TrackEnemyAlive>(SearchData))
		if (ensure(BTMemory->BlackboardObserverDelegateHandle.IsValid()))
			SearchData.OwnerComp.GetBlackboardComponent()->UnregisterObserver(ActiveTargetBBKey.GetSelectedKeyID(), BTMemory->BlackboardObserverDelegateHandle);
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

EBlackboardNotificationResult UBTDecorator_TrackEnemyAlive::OnEnemyChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	if (Key == ActiveTargetBBKey.GetSelectedKeyID())
	{
		auto NpcComponent = BlackboardComponent.GetBrainComponent()->GetAIOwner()->GetPawn()->FindComponentByClass<UNpcCombatLogicComponent>();
		auto NewTarget = Cast<AActor>(BlackboardComponent.GetValueAsObject(ActiveTargetBBKey.SelectedKeyName));
		if (NewTarget != nullptr)
			NpcComponent->TrackEnemyAlive(NewTarget);
		else
			NpcComponent->ResetTrackingEnemyAlive();
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_TrackEnemyAlive::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
		ActiveTargetBBKey.ResolveSelectedKey(*BB);
}

FString UBTDecorator_TrackEnemyAlive::GetStaticDescription() const
{
	return FString::Printf(TEXT("Observe if %s is alive"), *ActiveTargetBBKey.SelectedKeyName.ToString());
}
