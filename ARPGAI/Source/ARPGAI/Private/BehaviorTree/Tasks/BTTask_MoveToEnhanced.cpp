


#include "BehaviorTree/Tasks/BTTask_MoveToEnhanced.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_MoveToEnhanced::UBTTask_MoveToEnhanced(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Move To (enhanced)";
	ApproachAcceptanceRadiusBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToEnhanced, ApproachAcceptanceRadiusBBKey));
	ApproachChangeDistanceThresholdBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToEnhanced, ApproachChangeDistanceThresholdBBKey));
	ApproachAcceptanceRadiusBBKey.AllowNoneAsValue(true);
	ApproachChangeDistanceThresholdBBKey.AllowNoneAsValue(true);
}

void UBTTask_MoveToEnhanced::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		ApproachAcceptanceRadiusBBKey.ResolveSelectedKey(*BBAsset);
		ApproachChangeDistanceThresholdBBKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTTask_MoveToEnhanced::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nAcceptance Radius BB: %s\nObserved Blackboard Value Tolerance BB: %s"),
		*Super::GetStaticDescription(), *ApproachAcceptanceRadiusBBKey.SelectedKeyName.ToString(),
		*ApproachChangeDistanceThresholdBBKey.SelectedKeyName.ToString());
}

EBTNodeResult::Type UBTTask_MoveToEnhanced::PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		const float InitialAcceptanceRadius = AcceptableRadius.GetValue(Blackboard);
		const float InitialBlackboardValueTolerance = ObservedBlackboardValueTolerance.GetValue(Blackboard);
		const float InitialObserverBlackboardChanged = bObserveBlackboardValue;
		
		if (ApproachAcceptanceRadiusBBKey.IsSet())
		{
			AcceptableRadius = Blackboard->GetValueAsFloat(ApproachAcceptanceRadiusBBKey.SelectedKeyName);
		}

		bObserveBlackboardValue = ApproachChangeDistanceThresholdBBKey.IsSet();
		if (bObserveBlackboardValue)
		{
			if (ApproachChangeDistanceThresholdBBKey.IsSet())
			{
				ObservedBlackboardValueTolerance = Blackboard->GetValueAsFloat(ApproachChangeDistanceThresholdBBKey.SelectedKeyName);
			}
		}
		
		EBTNodeResult::Type Result = Super::PerformMoveTask(OwnerComp, NodeMemory);

		AcceptableRadius = InitialAcceptanceRadius;
		ObservedBlackboardValueTolerance = InitialBlackboardValueTolerance;
		bObserveBlackboardValue = InitialObserverBlackboardChanged;

		return Result;
	}
	else
	{
		return Super::PerformMoveTask(OwnerComp, NodeMemory);		
	}
}
