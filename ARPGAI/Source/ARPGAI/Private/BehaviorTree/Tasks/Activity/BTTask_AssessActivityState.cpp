#include "BehaviorTree/Tasks/Activity/BTTask_AssessActivityState.h"

#include "Activities/NpcComponentsHelpers.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "Interfaces/NpcControllerInterface.h"
#include "Interfaces/NpcGoalManager.h"

UBTTask_AssessActivityState::UBTTask_AssessActivityState()
{
	NodeName = "Assess activity state";
	bNotifyTick = 0;
}

EBTNodeResult::Type UBTTask_AssessActivityState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	INpcGoalManager* NpcActivityComponent = GetNpcGoalManager(OwnerComp);
	if (!ensure(NpcActivityComponent))
		return EBTNodeResult::Failed;

	bool bSuccess = NpcActivityComponent->SetActivityGoalData();
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_AssessActivityState::GetStaticDescription() const
{
	return TEXT("Load BT state for the next goal");
}
