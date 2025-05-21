// 


#include "BehaviorTree/Decorators/BTDecorator_NpcGoalControl.h"

#include "Activities/ActivityInstancesHelper.h"
#include "Components/Controller/NpcActivityComponent.h"

UBTDecorator_NpcGoalControl::UBTDecorator_NpcGoalControl()
{
	NodeName = "Npc goal control";
}

void UBTDecorator_NpcGoalControl::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (NodeResult == EBTNodeResult::Aborted)
		if (auto NpcActivityComponent = GetNpcActivityComponent(SearchData.OwnerComp))
			NpcActivityComponent->SuspendActiveGoal();
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}
