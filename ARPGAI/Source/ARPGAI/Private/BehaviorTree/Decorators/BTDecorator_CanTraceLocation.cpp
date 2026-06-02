#include "BehaviorTree/Decorators/BTDecorator_CanTraceLocation.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTDecorator_CanTraceLocation::UBTDecorator_CanTraceLocation()
{
	NodeName = "Can trace location";
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	TraceTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CanTraceLocation, TraceTargetBBKey), AActor::StaticClass());
	TraceTargetBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CanTraceLocation, TraceTargetBBKey));
}

bool UBTDecorator_CanTraceLocation::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	FVector ActualLocation = FAISystem::InvalidLocation;
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TraceActor = nullptr;
	
	if (TraceTargetBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		TraceActor = Cast<AActor>(Blackboard->GetValueAsObject(TraceTargetBBKey.SelectedKeyName));
		if (TraceActor)
			ActualLocation = TraceActor->GetActorLocation();
	}	
	else if (TraceTargetBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		ActualLocation = Blackboard->GetValueAsVector(TraceTargetBBKey.SelectedKeyName);
	}
	
	if (ActualLocation == FAISystem::InvalidLocation)
		return false;
	
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	FVector ViewLocation;
	FRotator ViewRotation;
	Pawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	FCollisionQueryParams CollisionQueryParams;
	ensure(CollisionQueryParams.bTraceComplex == false);
	ensure(CollisionQueryParams.bReturnPhysicalMaterial == false);
	CollisionQueryParams.AddIgnoredActor(Pawn);
	if (TraceActor)
		CollisionQueryParams.AddIgnoredActor(TraceActor);
	
	FHitResult HitResult;
	const float VerticalOffset = TraceEndOffsetZ.GetValue(Blackboard);
	bool bTraceBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, ActualLocation + FVector::UpVector * VerticalOffset,
		TraceChannel, CollisionQueryParams);

	return !bTraceBlocked;
}

void UBTDecorator_CanTraceLocation::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	if (FlowAbortMode == EBTFlowAbortMode::None)
		return;
	
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto Observer = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_CanTraceLocation::OnTraceTargetChanged);
	Blackboard->RegisterObserver(TraceTargetBBKey.GetSelectedKeyID(), this, Observer);
}

void UBTDecorator_CanTraceLocation::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (FlowAbortMode != EBTFlowAbortMode::None)
		if (auto Blackboard = OwnerComp.GetBlackboardComponent())
			Blackboard->UnregisterObserversFrom(this);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTDecorator_CanTraceLocation::OnTraceTargetChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	auto BTComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (ensure(Key == TraceTargetBBKey.GetSelectedKeyID()))
		ConditionalFlowAbort(*BTComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_CanTraceLocation::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	TraceTargetBBKey.ResolveSelectedKey(*Asset.GetBlackboardAsset());
}

FString UBTDecorator_CanTraceLocation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Check if can trace %s\n%s"),
		*TraceTargetBBKey.SelectedKeyName.ToString(), *Super::GetStaticDescription());
}
