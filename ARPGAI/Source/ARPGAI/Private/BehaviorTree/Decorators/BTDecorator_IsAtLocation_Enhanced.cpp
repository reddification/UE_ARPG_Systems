

#include "BehaviorTree/Decorators/BTDecorator_IsAtLocation_Enhanced.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"

UBTDecorator_IsAtLocation_Enhanced::UBTDecorator_IsAtLocation_Enhanced(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Is At Location (enhanced)";
	FlowAbortMode = EBTFlowAbortMode::Both;
	bAllowAbortLowerPri = true;
	bAllowAbortNone = true;
	bAllowAbortChildNodes = true;
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	AcceptableRangeBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsAtLocation_Enhanced, AcceptableRangeBBKey));
	AcceptableRangeBBKey.AllowNoneAsValue(true);
}

void UBTDecorator_IsAtLocation_Enhanced::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto BTMemory = GetNodeMemory<FBTMemory_IsAtLocationEnhanced>(SearchData);
	BTMemory->bIsDecoratorActive = true;
}

void UBTDecorator_IsAtLocation_Enhanced::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto BTMemory = GetNodeMemory<FBTMemory_IsAtLocationEnhanced>(SearchData))
		BTMemory->bIsDecoratorActive = false;
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}


bool UBTDecorator_IsAtLocation_Enhanced::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	// if (AcceptableRangeBBKey.IsNone())
	// 	return Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	AAIController* AIOwner = OwnerComp.GetAIOwner();
	float Radius = AcceptableRadius;
	if (!AcceptableRangeBBKey.IsNone())
		Radius = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(AcceptableRangeBBKey.SelectedKeyName);

	if (bAddNavAgentRadius)
	{
		if (auto Character = Cast<ACharacter>(AIOwner->GetPawn()))
			Radius += Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}
	
	bool bHasReached = false;

	if (UPathFollowingComponent* PathFollowingComponent = AIOwner ? AIOwner->GetPathFollowingComponent() : nullptr)
	{
		const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
		
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID());
			AActor* TargetActor = Cast<AActor>(KeyValue);
			if (TargetActor)
			{
				bHasReached = bPathFindingBasedTest.GetValue(MyBlackboard) 
					? PathFollowingComponent->HasReached(*TargetActor, EPathFollowingReachMode::OverlapAgentAndGoal, Radius, bUseNavAgentGoalLocation.GetValue(MyBlackboard))
					: (AIOwner->GetPawn()
						? (GetGeometricDistanceSquared(AIOwner->GetPawn()->GetActorLocation(), TargetActor->GetActorLocation()) < FMath::Square(Radius))
						: false);
			}
		}
		else if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			const FVector TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
			if (FAISystem::IsValidLocation(TargetLocation))
			{
				bHasReached = bPathFindingBasedTest.GetValue(MyBlackboard) 
					? PathFollowingComponent->HasReached(TargetLocation, EPathFollowingReachMode::OverlapAgent, Radius)
					: (AIOwner->GetPawn()
						? (GetGeometricDistanceSquared(AIOwner->GetPawn()->GetActorLocation(), TargetLocation) < FMath::Square(Radius))
						: false);
			}
		}
	}

	return bHasReached;
}

void UBTDecorator_IsAtLocation_Enhanced::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		AcceptableRangeBBKey.ResolveSelectedKey(*BBAsset);
	}
}

EBlackboardNotificationResult UBTDecorator_IsAtLocation_Enhanced::OnBlackboardKeyValueChange(
	const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	if (BehaviorComp == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}
	
	if (ChangedKeyID == BlackboardKey.GetSelectedKeyID())
	{
		int32 NodeIndex = BehaviorComp->FindInstanceContainingNode(this);
		uint8* RawMemory = BehaviorComp->GetNodeMemory(this, NodeIndex);
		auto BTMemory = reinterpret_cast<FBTMemory_IsAtLocationEnhanced*>(RawMemory);
		const bool bAtLocation = CalculateRawConditionValue(*BehaviorComp, RawMemory);
		const bool bInversed = IsInversed(); 
		if (!BTMemory->bIsDecoratorActive && bAtLocation != bInversed || BTMemory->bIsDecoratorActive && bAtLocation == bInversed)
		{
			BehaviorComp->RequestExecution(this);
		}
	}
	else
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

FString UBTDecorator_IsAtLocation_Enhanced::GetStaticDescription() const
{
	return !AcceptableRangeBBKey.IsNone()
		? FString::Printf(TEXT("%s\nAcceptable Distance BB key = %s\nObserves target"), *Super::GetStaticDescription(), *AcceptableRangeBBKey.SelectedKeyName.ToString())
		: FString::Printf(TEXT("%s\nAcceptable Distance = %.2f\nObserves target"), *Super::GetStaticDescription(), AcceptableRadius);
}