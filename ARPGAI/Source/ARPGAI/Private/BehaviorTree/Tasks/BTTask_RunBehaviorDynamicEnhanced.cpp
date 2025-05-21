

#include "BehaviorTree/Tasks/BTTask_RunBehaviorDynamicEnhanced.h"
#include "AIController.h"
#include "Interfaces/NpcControllerInterface.h"

UBTTask_RunBehaviorDynamicEnhanced::UBTTask_RunBehaviorDynamicEnhanced()
{
	NodeName = "Run behavior dynamic enhanced";
}

EBTNodeResult::Type UBTTask_RunBehaviorDynamicEnhanced::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	if (Result != EBTNodeResult::Failed)
	{
		if (INpcControllerInterface* NpcController = Cast<INpcControllerInterface>(OwnerComp.GetAIOwner()))
		{
			const int32 InstanceStackStartIndex = OwnerComp.GetActiveInstanceIdx();
			NpcController->InjectDynamicBehaviors(InstanceStackStartIndex);	
		}
		else
		{
			Result = EBTNodeResult::Failed;
		}
	}

	return Result;
}

FString UBTTask_RunBehaviorDynamicEnhanced::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nSupports recursive injection"), *Super::GetStaticDescription());
}
