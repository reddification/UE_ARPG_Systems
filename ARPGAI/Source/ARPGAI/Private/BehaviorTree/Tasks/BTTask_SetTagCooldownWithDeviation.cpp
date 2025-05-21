


#include "BehaviorTree/Tasks/BTTask_SetTagCooldownWithDeviation.h"

UBTTask_SetTagCooldownWithDeviation::UBTTask_SetTagCooldownWithDeviation()
{
	NodeName = "Set Tag Cooldown with deviation";
}

EBTNodeResult::Type UBTTask_SetTagCooldownWithDeviation::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                                     uint8* NodeMemory)
{
	float ActualCooldownTime = FMath::Max(0.f,  CooldownDuration + FMath::RandRange(-DeviationTime, +DeviationTime));
	OwnerComp.AddCooldownTagDuration(CooldownTag, ActualCooldownTime, bAddToExistingDuration);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_SetTagCooldownWithDeviation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Set cooldown for tag %s in range [%.2fs; %.2fs]"), *CooldownTag.ToString(),
		FMath::Max(0, CooldownDuration - DeviationTime), CooldownDuration + DeviationTime);
}