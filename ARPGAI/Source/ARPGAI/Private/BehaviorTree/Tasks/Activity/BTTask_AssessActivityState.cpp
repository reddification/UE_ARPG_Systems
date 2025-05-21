#include "BehaviorTree/Tasks/Activity/BTTask_AssessActivityState.h"

#include "Activities/ActivityInstancesHelper.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "Components/Controller/NpcActivityComponent.h"
#include "Interfaces/NpcControllerInterface.h"

UBTTask_AssessActivityState::UBTTask_AssessActivityState()
{
	NodeName = "Assess activity state";
	bNotifyTick = 0;
}

EBTNodeResult::Type UBTTask_AssessActivityState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto NpcActivityComponent = GetNpcActivityComponent(OwnerComp);
	if (!ensure(NpcActivityComponent))
		return EBTNodeResult::Failed;

	bool bSuccess = NpcActivityComponent->SetActivityGoalData();
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_AssessActivityState::GetStaticDescription() const
{
	return TEXT("Load BT state for the next goal");
}
