


#include "BehaviorTree/Decorators/BTDecorator_IsCircumventionRequired.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsCircumventionRequired::UBTDecorator_IsCircumventionRequired()
{
	NodeName = "Is circumvention required";
	TargetActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsCircumventionRequired, TargetActorBBKey), AActor::StaticClass());
	DestinationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsCircumventionRequired, DestinationBBKey));
	bNotifyBecomeRelevant = 1;
	bNotifyCeaseRelevant = 1;
	FlowAbortMode = EBTFlowAbortMode::Type::Both;
}

void UBTDecorator_IsCircumventionRequired::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = Asset.GetBlackboardAsset())
	{
		DestinationBBKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTDecorator_IsCircumventionRequired::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
	{
		auto KeyID = DestinationBBKey.GetSelectedKeyID();
		BlackboardComp->RegisterObserver(KeyID, this,
			FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_IsCircumventionRequired::OnDestinationChanged));
	}
}

void UBTDecorator_IsCircumventionRequired::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
	{
		BlackboardComp->UnregisterObserversFrom(this);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTDecorator_IsCircumventionRequired::OnDestinationChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey KeyId)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (BehaviorComp == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	uint8* RawMemory = BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(this));
	if (DestinationBBKey.GetSelectedKeyID() == KeyId && CalculateRawConditionValue(*BehaviorComp, RawMemory))
	{
		BehaviorComp->RequestExecution(this);		
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}

bool UBTDecorator_IsCircumventionRequired::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	const AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorBBKey.SelectedKeyName));
	if (IsValid(TargetActor) == false)
	{
		return false;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector Destination = Blackboard->GetValueAsVector(DestinationBBKey.SelectedKeyName);
	const float DotProduct = FVector::DotProduct((AIPawn->GetActorLocation() - TargetLocation).GetSafeNormal(),
		(Destination - TargetLocation).GetSafeNormal());
	return DotProduct < DotProductThreshold;
}

FString UBTDecorator_IsCircumventionRequired::GetStaticDescription() const
{
	return FString::Printf(TEXT("Target actor BB: %s\nDestination BB: %s\nDot product threshold: [%.2f; %.2f]"),
		*TargetActorBBKey.SelectedKeyName.ToString(), *DestinationBBKey.SelectedKeyName.ToString(), -1.f, DotProductThreshold);
}
