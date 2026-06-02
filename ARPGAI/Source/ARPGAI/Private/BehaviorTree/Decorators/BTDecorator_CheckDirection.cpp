#include "BehaviorTree/Decorators/BTDecorator_CheckDirection.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTDecorator_CheckDirection::UBTDecorator_CheckDirection()
{
	NodeName = "Check direction";
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckDirection, TargetBBKey), AActor::StaticClass());
	TargetBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckDirection, TargetBBKey));
}

bool UBTDecorator_CheckDirection::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	FVector ActualLocation = FAISystem::InvalidLocation;
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	
	if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		if (auto TraceActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName)))
			ActualLocation = TraceActor->GetActorLocation();
	}	
	else if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		ActualLocation = Blackboard->GetValueAsVector(TargetBBKey.SelectedKeyName);
	}
	
	if (ActualLocation == FAISystem::InvalidLocation)
		return false;
	
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	const float AngleRotation = ViewDirectionYawOffset.GetValue(Blackboard);
	FVector PawnVector = Pawn->GetActorForwardVector();
	if (!FMath::IsNearlyZero(AngleRotation)) 
		PawnVector = PawnVector.RotateAngleAxis(AngleRotation, Pawn->GetActorUpVector());
	
	FVector Direction = (ActualLocation - Pawn->GetActorLocation()).GetSafeNormal();
	const float DP = Direction | PawnVector;
	
	return DP >= FMath::Cos(FMath::DegreesToRadians(AngleThreshold.GetValue(Blackboard)));
}

void UBTDecorator_CheckDirection::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	if (FlowAbortMode == EBTFlowAbortMode::None)
		return;
	
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto Observer = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_CheckDirection::OnTargetChanged);
	Blackboard->RegisterObserver(TargetBBKey.GetSelectedKeyID(), this, Observer);
}

void UBTDecorator_CheckDirection::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (FlowAbortMode != EBTFlowAbortMode::None)
		if (auto Blackboard = OwnerComp.GetBlackboardComponent())
			Blackboard->UnregisterObserversFrom(this);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTDecorator_CheckDirection::OnTargetChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	auto BTComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (ensure(Key == TargetBBKey.GetSelectedKeyID()))
		ConditionalFlowAbort(*BTComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_CheckDirection::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	TargetBBKey.ResolveSelectedKey(*Asset.GetBlackboardAsset());
}

FString UBTDecorator_CheckDirection::GetStaticDescription() const
{
	return FString::Printf(TEXT("Check if %s is in %s cone angle\nYaw offset: %s\n%s"),
		*TargetBBKey.SelectedKeyName.ToString(), *AngleThreshold.ToString(), *ViewDirectionYawOffset.ToString(), *Super::GetStaticDescription());
}
