// 


#include "BehaviorTree/Decorators/BTDecorator_IsCharacterMoving.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTDecorator_IsCharacterMoving::UBTDecorator_IsCharacterMoving()
{
	NodeName = "Is character moving";
	CharacterBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsCharacterMoving, CharacterBBKey), AActor::StaticClass());

	bNotifyTick = true;
	bTickIntervals = true;

	bNotifyBecomeRelevant = true;
	FlowAbortMode = EBTFlowAbortMode::Both;
}

void UBTDecorator_IsCharacterMoving::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	SetNextTickTime(NodeMemory, FlowAbortMode != EBTFlowAbortMode::None ? TickInterval : FLT_MAX);
}

void UBTDecorator_IsCharacterMoving::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	ConditionalFlowAbort(OwnerComp, EBTDecoratorAbortRequest::ConditionResultChanged);
	SetNextTickTime(NodeMemory, TickInterval);
}

bool UBTDecorator_IsCharacterMoving::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
                                                                uint8* NodeMemory) const
{
	auto Actor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(CharacterBBKey.SelectedKeyName));
	if (Actor == nullptr)
		return false;

	auto Pawn = Cast<APawn>(Actor);
	if (Pawn == nullptr)
		return Actor->GetVelocity().SizeSquared2D() > 20.f * 20.f;

	if (auto AIController = Cast<AAIController>(Pawn->GetController()))
	{
		auto PathFollowingComponent = AIController->GetPathFollowingComponent();
		return PathFollowingComponent->GetStatus() != EPathFollowingStatus::Idle && PathFollowingComponent->HasValidPath();
	}
	else if (auto PlayerController = Cast<APlayerController>(Pawn->GetController()))
	{
		return Actor->GetVelocity().SizeSquared2D() > 20.f * 20.f;
	}

	return false;
}

FString UBTDecorator_IsCharacterMoving::GetStaticDescription() const
{
	return FString::Printf(TEXT("Check if %s is moving\nUpdate interval = %.2f\n%s"), *CharacterBBKey.SelectedKeyName.ToString(), TickInterval, *Super::GetStaticDescription());
}
