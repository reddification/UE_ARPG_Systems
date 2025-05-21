// 


#include "BehaviorTree/Decorators/BTDecorator_UpdateSpeedToReachLocationInTime.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcSettings.h"
#include "Interfaces/Npc.h"

UBTDecorator_UpdateSpeedToReachLocationInTime::UBTDecorator_UpdateSpeedToReachLocationInTime()
{
	NodeName = "Update speed to reach location in time";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	TargetLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_UpdateSpeedToReachLocationInTime, TargetLocationBBKey));
}

void UBTDecorator_UpdateSpeedToReachLocationInTime::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	auto BTMemory = GetNodeMemory<FBTMemory_UpdateSpeedToReachLocationInTime>(SearchData);
	auto ObserverHandler = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_UpdateSpeedToReachLocationInTime::OnTargetChanged);
	BTMemory->BlackboardKeyChangedDelegateHandle = Blackboard->RegisterObserver(TargetLocationBBKey.GetSelectedKeyID(), this, ObserverHandler);
	UpdateMoveSpeed(*Blackboard);
}

void UBTDecorator_UpdateSpeedToReachLocationInTime::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto Npc = Cast<INpc>(SearchData.OwnerComp.GetAIOwner()->GetPawn()))
		Npc->ResetForcedMoveSpeed();

	auto BTMemory = GetNodeMemory<FBTMemory_UpdateSpeedToReachLocationInTime>(SearchData);
	if (BTMemory && BTMemory->BlackboardKeyChangedDelegateHandle.IsValid())
	{
		auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
		Blackboard->UnregisterObserver(TargetLocationBBKey.GetSelectedKeyID(), BTMemory->BlackboardKeyChangedDelegateHandle);
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

void UBTDecorator_UpdateSpeedToReachLocationInTime::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		TargetLocationBBKey.ResolveSelectedKey(*BB);
	}

	TimeToGetAtLocation = GetDefault<UNpcSettings>()->FollowLeaderPredictionTime; 
}

void UBTDecorator_UpdateSpeedToReachLocationInTime::UpdateMoveSpeed(const UBlackboardComponent& BlackboardComponent)
{
	const FVector CurrentTargetLocation = BlackboardComponent.GetValueAsVector(TargetLocationBBKey.SelectedKeyName);
	auto Pawn = BlackboardComponent.GetBrainComponent()->GetAIOwner()->GetPawn();
	const float NewForcedMoveSpeed = (Pawn->GetActorLocation() - CurrentTargetLocation).Size() / TimeToGetAtLocation;
	const float ClampedNewForcedMoveSpeed = FMath::Clamp(NewForcedMoveSpeed, MinSpeed, MaxSpeed);
	auto Npc = Cast<INpc>(Pawn);
	Npc->SetForcedMoveSpeed(ClampedNewForcedMoveSpeed);
}

EBlackboardNotificationResult UBTDecorator_UpdateSpeedToReachLocationInTime::OnTargetChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	if (Key == TargetLocationBBKey.GetSelectedKeyID())
	{
		UpdateMoveSpeed(BlackboardComponent);
		return EBlackboardNotificationResult::ContinueObserving;
	}

	return EBlackboardNotificationResult::RemoveObserver;
}

FString UBTDecorator_UpdateSpeedToReachLocationInTime::GetStaticDescription() const
{
	return FString::Printf(TEXT("Update NPC speed when %s is updated\nSpeed is clamped [%.2f -> %.2f]"), *TargetLocationBBKey.SelectedKeyName.ToString(),
		MinSpeed, MaxSpeed);
}
