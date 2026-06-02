#include "BehaviorTree/Tasks/Combat/BTTask_DropWeapon.h"

#include "AIController.h"
#include "Interfaces/NpcWeaponInterface.h"

UBTTask_DropWeapon::UBTTask_DropWeapon()
{
	NodeName = "Drop weapon";
}

EBTNodeResult::Type UBTTask_DropWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Npc = Cast<INpcWeaponInterface>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		Npc->DropUnsheathedWeapon_NPC();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
