// 


#include "BehaviorTree/Services/BTService_UpdateTargetReachable.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
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
		ensure(false);
		return;
	}
	
	const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwnerComp.GetWorld());
	if (!ensure(NavSys))
		return;

	
	bool bHasPath = false;

	const AAIController* AIOwner = OwnerComp.GetAIOwner();
	auto PawnLocation = AIOwner->GetPawn()->GetActorLocation();
	const ANavigationData* NavData = AIOwner ? NavSys->GetNavDataForProps(AIOwner->GetNavAgentPropertiesRef(), AIOwner->GetNavAgentLocation()) : NULL;
	if (NavData)
	{
		FSharedConstNavQueryFilter QueryFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, AIOwner, AIOwner->GetDefaultNavigationFilterClass());
		EPathFindingMode::Type TestMode = bUseHierarchicalPathfinding ? EPathFindingMode::Hierarchical : EPathFindingMode::Regular;
		bHasPath = NavSys->TestPathSync(FPathFindingQuery(AIOwner, *NavData, PawnLocation, TargetActor->GetActorLocation(), QueryFilter), TestMode);
	}

	Blackboard->SetValueAsBool(OutIsTargetUnreachableBBKey.SelectedKeyName, !bHasPath);
}

void UBTService_UpdateTargetReachable::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_UpdateTargetReachable::OnTargetChanged);
	Blackboard->RegisterObserver(TargetBBKey.GetSelectedKeyID(), this, ObserverDelegate);
	if (Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName) == nullptr)
		SetNextTickTime(NodeMemory, FLT_MAX);
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
	return FString::Printf(TEXT("Target BB:%s\n[out]Target unreachable BB:%s\n%s"),
		*TargetBBKey.SelectedKeyName.ToString(), *OutIsTargetUnreachableBBKey.SelectedKeyName.ToString(),
		*Super::GetStaticDescription());
}
