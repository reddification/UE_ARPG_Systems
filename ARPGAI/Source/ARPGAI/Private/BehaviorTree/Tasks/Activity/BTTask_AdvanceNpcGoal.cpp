#include "BehaviorTree/Tasks/Activity/BTTask_AdvanceNpcGoal.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "Activities/ActivityInstancesHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcActivityComponent.h"

UBTTask_AdvanceNpcGoal::UBTTask_AdvanceNpcGoal()
{
	NodeName = "Advance Npc Goal";
	OutGoalAdvancedBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AdvanceNpcGoal, OutGoalAdvancedBBKey));
	GoalExecutionResultBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_AdvanceNpcGoal, GoalExecutionResultBBKey)));
}

EBTNodeResult::Type UBTTask_AdvanceNpcGoal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto BlackboardComponent = OwnerComp.GetBlackboardComponent();
	auto NpcActivityComponent = GetNpcActivityComponent(OwnerComp);
	if (!ensure(NpcActivityComponent))
	{
		BlackboardComponent->SetValueAsBool(OutGoalAdvancedBBKey.SelectedKeyName, false);
		BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(GoalExecutionResultBBKey.SelectedKeyName, FGameplayTagContainer::EmptyContainer);
		return EBTNodeResult::Failed;
	}

	FGameplayTagContainer GoalExecutionResultTags = BlackboardComponent->GetValue<UBlackboardKeyType_GameplayTag>(GoalExecutionResultBBKey.SelectedKeyName);
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(GoalExecutionResultBBKey.SelectedKeyName, FGameplayTagContainer::EmptyContainer);
	ENpcGoalAdvanceResult NpcGoalAdvanceResult = NpcActivityComponent->AdvanceCurrentGoal(bGoalSucceeded, GoalExecutionResultTags);
	if (NpcGoalAdvanceResult == ENpcGoalAdvanceResult::InProgress)
	{
		BlackboardComponent->SetValueAsBool(OutGoalAdvancedBBKey.SelectedKeyName, true);
		BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(GoalExecutionResultBBKey.SelectedKeyName, FGameplayTagContainer::EmptyContainer);
		return EBTNodeResult::Succeeded;
	}
	
	bool bSuccess = NpcActivityComponent->RequestNextNpcGoal();
	if (bSuccess)
		NpcActivityComponent->SetActivityGoalData();
	
	BlackboardComponent->SetValueAsBool(OutGoalAdvancedBBKey.SelectedKeyName, bSuccess);
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_AdvanceNpcGoal::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\n[out]Goal assessed BB: %s\nGoal execution result BB key: %s"),
		bGoalSucceeded ? TEXT("Goal succeeded") : TEXT("Goal failed"),
		*OutGoalAdvancedBBKey.SelectedKeyName.ToString(), *GoalExecutionResultBBKey.SelectedKeyName.ToString());
}
