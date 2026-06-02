#include "BehaviorTree/Decorators/BTDecorator_IsPointWithinNpcArea.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcComponent.h"

UBTDecorator_IsPointWithinNpcArea::UBTDecorator_IsPointWithinNpcArea()
{
	NodeName = "Is point within combat zone";
	LocationBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsPointWithinNpcArea, LocationBBKey), AActor::StaticClass());
	LocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsPointWithinNpcArea, LocationBBKey));
	bNotifyBecomeRelevant = FlowAbortMode != EBTFlowAbortMode::None;
	bNotifyCeaseRelevant = FlowAbortMode != EBTFlowAbortMode::None;
}

bool UBTDecorator_IsPointWithinNpcArea::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	auto NpcComponent = GetNpcAreasComponent(OwnerComp);
	if (NpcComponent == nullptr)
		return false;

	FVector Location = FAISystem::InvalidLocation;

	if (LocationBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		auto Actor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(LocationBBKey.SelectedKeyName));
		if (Actor != nullptr)
			Location = Actor->GetActorLocation();
	}
	else if (LocationBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		Location = OwnerComp.GetBlackboardComponent()->GetValueAsVector(LocationBBKey.SelectedKeyName);
	}

	if (Location == FAISystem::InvalidLocation)
		return false;
	
	if (!NpcComponent->HasAreas())
		return bResultIfNpcHasNoTerritory;
	
	return NpcComponent->IsLocationWithinNpcArea(Location, Extent);
}

void UBTDecorator_IsPointWithinNpcArea::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto BlackboardObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_IsPointWithinNpcArea::OnLocationChanged);
	Blackboard->RegisterObserver(LocationBBKey.GetSelectedKeyID(), this, BlackboardObserverDelegate);
}

void UBTDecorator_IsPointWithinNpcArea::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
		Blackboard->UnregisterObserversFrom(this);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTDecorator_IsPointWithinNpcArea::OnLocationChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	auto BTComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (ensure(Key == LocationBBKey.GetSelectedKeyID()))	
		ConditionalFlowAbort(*BTComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_IsPointWithinNpcArea::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		LocationBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTDecorator_IsPointWithinNpcArea::GetStaticDescription() const
{
	return FString::Printf(TEXT("Location BB: %s\nTest extent = %.2f"), *LocationBBKey.SelectedKeyName.ToString(), Extent);
}
