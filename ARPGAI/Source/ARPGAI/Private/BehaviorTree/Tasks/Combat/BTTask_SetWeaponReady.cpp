#include "BehaviorTree/Tasks/Combat/BTTask_SetWeaponReady.h"

#include "AIController.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/NpcWeaponInterface.h"

UBTTask_SetWeaponReady::UBTTask_SetWeaponReady()
{
	NodeName = "Set weapon ready";
}

EBTNodeResult::Type UBTTask_SetWeaponReady::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto NPC = Cast<INpcWeaponInterface>(OwnerComp.GetAIOwner()->GetPawn());
	if (ensure(NPC))
	{
		Super::ExecuteTask(OwnerComp, NodeMemory);
		bool bAlreadyInRequestedState = NPC->RequestWeaponReady_NPC(bSetReady);
		if (!bAlreadyInRequestedState && bAwaitCompletion)
			return EBTNodeResult::InProgress;

		return EBTNodeResult::Succeeded;
	}

	return bForceSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_SetWeaponReady::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto NPC = Cast<INpcWeaponInterface>(OwnerComp.GetAIOwner()->GetPawn()))
		NPC->CancelWeaponReady_NPC(bSetReady);
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_SetWeaponReady::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_SetWeaponReady_Completed;
}

FString UBTTask_SetWeaponReady::GetStaticDescription() const
{
	FString Result = bSetReady ? TEXT("Unsheathe weapon") : TEXT("Sheathe weapon");
	
	if (bAwaitCompletion)
		Result += TEXT("\nWait for completion");

	if (bForceSuccess)
		Result += TEXT("\nForce success");
	
	return Result;
}
