#include "BehaviorTree/Decorators/BTDecorator_FollowActor.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"
#include "Navigation/PathFollowingComponent.h"

UBTDecorator_FollowActor::UBTDecorator_FollowActor()
{
	NodeName = "Follow actor";
	bNotifyActivation = true;
	bNotifyDeactivation = true;

	FlowAbortMode = EBTFlowAbortMode::Self;

	bAllowAbortChildNodes = true;
	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	
	FollowTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_FollowActor, FollowTargetBBKey), AActor::StaticClass());
}

bool UBTDecorator_FollowActor::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	return Blackboard->GetValueAsObject(FollowTargetBBKey.SelectedKeyName) != nullptr;
	
	// this decorator shouldnt have the authority to decide if NPC should follow the target
	// but this logic could probably be useful somewhere else...
	// TODO (aki) 28.01.2026: decide what to do with it
	// return IsNeedToFollowTarget(Blackboard);
}

bool UBTDecorator_FollowActor::IsNeedToFollowTarget(UBlackboardComponent* Blackboard) const
{
	auto Actor = Cast<AActor>(Blackboard->GetValueAsObject(FollowTargetBBKey.SelectedKeyName));
	if (Actor == nullptr)
		return false;

	auto Pawn = Cast<APawn>(Actor);
	if (Pawn == nullptr)
	{
		UE_VLOG(Blackboard->GetOwner(), LogARPGAI, Warning, TEXT("UBTDecorator_FollowActor::IsNeedToFollowTarget: Following an actor which is not a pawn. Not intended behavior"));
		return Actor->GetVelocity().SizeSquared2D() > 20.f * 20.f;
	}
	
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

void UBTDecorator_FollowActor::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto FollowTarget = Cast<AActor>(SearchData.OwnerComp.GetBlackboardComponent()->GetValueAsObject(FollowTargetBBKey.SelectedKeyName));
	auto NpcComponent = GetNpcComponent(SearchData.OwnerComp);
	NpcComponent->SetFollowTarget(FollowTarget);
}

void UBTDecorator_FollowActor::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	if (NodeResult != EBTNodeResult::Aborted)
	{
		if (auto DecoratorParentNode = GetParentNode())
		{
			if (IsNeedToFollowTarget(SearchData.OwnerComp.GetBlackboardComponent()))
			{
				DecoratorParentNode->SetChildOverride(SearchData, GetChildIndex());
				return;
			}
		}
	}

	if (auto NpcComponent = GetNpcComponent(SearchData.OwnerComp))
		NpcComponent->ClearFollowTarget();
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_FollowActor::GetStaticDescription() const
{
	return FString::Printf(TEXT("Follow %s until it stops\n%s"), *FollowTargetBBKey.SelectedKeyName.ToString(), *Super::GetStaticDescription());
}