#include "BehaviorTree/Services/BTService_AreaRubberband.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcAreasComponent.h"

UBTService_AreaRubberband::UBTService_AreaRubberband()
{
	NodeName = "Area Rubberband";
	OutRubberbandingActiveBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_AreaRubberband, OutRubberbandingActiveBBKey));
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_AreaRubberband::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto BTMemory = reinterpret_cast<FBTMemory_AreaRubberband*>(NodeMemory);
	ensure(BTMemory->NpcAreasComponent->HasAreas());
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto OwnerLocation = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
	bool bInArea = BTMemory->NpcAreasComponent->IsLocationWithinNpcArea(OwnerLocation, AreaExtent.GetValue(Blackboard));
	bool bAlreadyActive = Blackboard->GetValueAsBool(OutRubberbandingActiveBBKey.SelectedKeyName);
	
	if (!bInArea && !bAlreadyActive)
	{
		BTMemory->OutOfAreaDuration += DeltaSeconds;
		if (!bAlreadyActive && BTMemory->OutOfAreaDuration > ActivationDelay.GetValue(Blackboard))
			Blackboard->SetValueAsBool(OutRubberbandingActiveBBKey.SelectedKeyName, true);
	}
	else if (bInArea && bAlreadyActive)
	{
		BTMemory->OutOfAreaDuration = 0.f;
		if (bAlreadyActive)
			Blackboard->SetValueAsBool(OutRubberbandingActiveBBKey.SelectedKeyName, false);
	}
}

void UBTService_AreaRubberband::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto BTMemory = reinterpret_cast<FBTMemory_AreaRubberband*>(NodeMemory);
	BTMemory->NpcAreasComponent = GetNpcAreasComponent(OwnerComp);;
	if (!BTMemory->NpcAreasComponent.IsValid() || !BTMemory->NpcAreasComponent->HasAreas())
		SetNextTickTime(NodeMemory, FLT_MAX);
	else 
		SetNextTickTime(NodeMemory, 0.f);
}

void UBTService_AreaRubberband::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
		Blackboard->ClearValue(OutRubberbandingActiveBBKey.SelectedKeyName);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

FString UBTService_AreaRubberband::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out]Rubberband active BB: %s\nActivation delay = %s\nArea extent = %s\n%s"),
		*OutRubberbandingActiveBBKey.SelectedKeyName.ToString(), *ActivationDelay.ToString(), *AreaExtent.ToString(),
		*Super::GetStaticDescription());
}
