


#include "BehaviorTree/Tasks/BTTask_SetTagCooldownWithDeviation.h"

UBTTask_SetTagCooldownWithDeviation::UBTTask_SetTagCooldownWithDeviation()
{
	NodeName = "Set Tag Cooldown with deviation";
}

EBTNodeResult::Type UBTTask_SetTagCooldownWithDeviation::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                                     uint8* NodeMemory)
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	float ActualCooldownTime = FMath::Max(0.f,  CooldownDuration.GetValue(Blackboard) + FMath::RandRange(-DeviationTime, +DeviationTime));
	OwnerComp.AddCooldownTagDuration(CooldownTag, ActualCooldownTime, bAddToExistingDuration);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_SetTagCooldownWithDeviation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Set cooldown for tag %s in range %s +- %.2fs"), *CooldownTag.ToString(),
		*CooldownDuration.ToString(), DeviationTime);
}