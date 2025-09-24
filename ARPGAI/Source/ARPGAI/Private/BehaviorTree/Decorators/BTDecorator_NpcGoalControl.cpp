// 


#include "BehaviorTree/Decorators/BTDecorator_NpcGoalControl.h"

#include "AIController.h"
#include "Interfaces/NpcGoalManager.h"

UBTDecorator_NpcGoalControl::UBTDecorator_NpcGoalControl()
{
	NodeName = "Npc goal control";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_NpcGoalControl::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (auto GoalManager = Cast<INpcGoalManager>(SearchData.OwnerComp.GetAIOwner()))
		GoalManager->ResumeGoal();
}

void UBTDecorator_NpcGoalControl::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
                                                     EBTNodeResult::Type NodeResult)
{
	if (NodeResult == EBTNodeResult::Aborted)
		if (auto GoalManager = Cast<INpcGoalManager>(SearchData.OwnerComp.GetAIOwner()))
			GoalManager->SuspendActiveGoal();
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}
