


#include "BehaviorTree/Tasks/BTTask_RunBehaviorEnhanced.h"
#include "AIController.h"
#include "Interfaces/NpcControllerInterface.h"

UBTTask_RunBehaviorEnhanced::UBTTask_RunBehaviorEnhanced()
{
	NodeName = "Run behavior enhanced";
}

EBTNodeResult::Type UBTTask_RunBehaviorEnhanced::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	if (Result != EBTNodeResult::Failed)
	{
		if (INpcControllerInterface* AIController = Cast<INpcControllerInterface>(OwnerComp.GetAIOwner()))
		{
			AIController->InjectDynamicBehaviors(GetParentNode());	
		}
		else
		{
			Result = EBTNodeResult::Failed;
		}
	}

	return Result;
}

FString UBTTask_RunBehaviorEnhanced::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nSupports recursive injection"), *Super::GetStaticDescription());
}
