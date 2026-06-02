// 


#include "BehaviorTree/Services/BTService_UpdateTargetReachable.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/LogChannels.h"
#include "NavFilters/NavigationQueryFilter.h"

UBTService_UpdateTargetReachable::UBTService_UpdateTargetReachable()
{
	NodeName = "Check if target reachable";
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTargetReachable, TargetBBKey), AActor::StaticClass());
	OutIsTargetUnreachableBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTargetReachable, OutIsTargetUnreachableBBKey));
}

void UBTService_UpdateTargetReachable::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
	if (TargetActor == nullptr)
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Warning, TEXT("UBTService_UpdateTargetReachable: Target is nullptr"));
		Blackboard->SetValueAsBool(OutIsTargetUnreachableBBKey.SelectedKeyName, false);
		return;
	}
	
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwnerComp.GetWorld());
	if (!ensure(NavSys))
		return;

	bool bHasPath = false;

	auto BTMemory = reinterpret_cast<FBTMemory_UpdateTargetReachable*>(NodeMemory);
	
	const AAIController* AIOwner = OwnerComp.GetAIOwner();
	auto PawnLocation = AIOwner->GetPawn()->GetActorLocation();
	const ANavigationData* NavData = AIOwner ? NavSys->GetNavDataForProps(AIOwner->GetNavAgentPropertiesRef(), AIOwner->GetNavAgentLocation()) : NULL;
	if (NavData)
	{
		FSharedConstNavQueryFilter QueryFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, AIOwner, AIOwner->GetDefaultNavigationFilterClass());
		EPathFindingMode::Type TestMode = bUseHierarchicalPathfinding ? EPathFindingMode::Hierarchical : EPathFindingMode::Regular;
		bHasPath = NavSys->TestPathSync(FPathFindingQuery(AIOwner, *NavData, PawnLocation, TargetActor->GetActorLocation(), QueryFilter), TestMode);
	}

	BTMemory->ConsequitiveUnreachableChecks = bHasPath ? 0 : BTMemory->ConsequitiveUnreachableChecks + 1;
	Blackboard->SetValueAsBool(OutIsTargetUnreachableBBKey.SelectedKeyName, BTMemory->ConsequitiveUnreachableChecks >= MinUnreachableChecksCount);
}

void UBTService_UpdateTargetReachable::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_UpdateTargetReachable::OnTargetChanged);
	Blackboard->RegisterObserver(TargetBBKey.GetSelectedKeyID(), this, ObserverDelegate);
	if (Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName) == nullptr)
		SetNextTickTime(NodeMemory, FLT_MAX);
	
	// 26 Apr 2026 (aki): just checking hypothesis. What's happening is that after NPC looses target (e.g. target runs behind an obstacle) 
	// this service doesn't tick when target reappears. My assumption: SetNextTickTime in blackboard observer callback is effective even after OnCeaseRelevant
	// if (GetNextTickRemainingTime(NodeMemory) > 1000.f) 
	// upd: yes. if SetNextTickTime(NodeMemory, FLT_MAX) was called - then you have to call SetNextTickTime(NodeMemory, 0) again, otherwise the service won't tick
	SetNextTickTime(NodeMemory, 0.f);
	
	auto BTMemory = reinterpret_cast<FBTMemory_UpdateTargetReachable*>(NodeMemory);
	BTMemory->ConsequitiveUnreachableChecks = 0;
}

void UBTService_UpdateTargetReachable::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	if (IsValid(Blackboard))
		Blackboard->UnregisterObserversFrom(this);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTService_UpdateTargetReachable::OnTargetChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	auto BTComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	auto NodeMemory = BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this));
	auto BTMemory = reinterpret_cast<FBTMemory_UpdateTargetReachable*>(NodeMemory);
	BTMemory->ConsequitiveUnreachableChecks = 0;
	if (BlackboardComponent.GetValueAsObject(TargetBBKey.SelectedKeyName) == nullptr)
		SetNextTickTime(NodeMemory, FLT_MAX);
	else
		SetNextTickTime(NodeMemory, Interval);

	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTService_UpdateTargetReachable::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		TargetBBKey.ResolveSelectedKey(*BB);
		OutIsTargetUnreachableBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTService_UpdateTargetReachable::GetStaticDescription() const
{
	return FString::Printf(TEXT("Target BB:%s\n[out]Target unreachable BB:%s\nUnreachable after %d PF checks failed in a row\n%s"),
		*TargetBBKey.SelectedKeyName.ToString(), *OutIsTargetUnreachableBBKey.SelectedKeyName.ToString(), MinUnreachableChecksCount,
		*Super::GetStaticDescription());
}
