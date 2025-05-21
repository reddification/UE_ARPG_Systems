


#include "BehaviorTree/Services/BTService_EvaluateApproachUtility.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTService_EvaluateApproachUtility::UBTService_EvaluateApproachUtility()
{
	NodeName = "Evaluate approach utility";
	StraightforwardUtilityBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateApproachUtility, StraightforwardUtilityBBKey));
	ManeuverUtilityBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateApproachUtility, ManeuverUtilityBBKey));
	// bNotifyBecomeRelevant = 1;
	bCallTickOnSearchStart = 1;
}

void UBTService_EvaluateApproachUtility::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	EvaluateUtility(OwnerComp, NodeMemory);
}

void UBTService_EvaluateApproachUtility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	EvaluateUtility(OwnerComp, NodeMemory);
}

void UBTService_EvaluateApproachUtility::EvaluateUtility(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	// TODO add logic for actual evaluation
	Blackboard->SetValueAsFloat(StraightforwardUtilityBBKey.SelectedKeyName, 1.f);
	Blackboard->SetValueAsFloat(ManeuverUtilityBBKey.SelectedKeyName, 0.5f);
}

FString UBTService_EvaluateApproachUtility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Straightforward utility BB Key: %s\nManeuver utility BB Key: %s\n%s"),
		*StraightforwardUtilityBBKey.SelectedKeyName.ToString(), *ManeuverUtilityBBKey.SelectedKeyName.ToString(),
		*Super::GetStaticDescription());
}
