#include "BehaviorTree/Tasks/Actions/BTTask_EquipBestWeapon.h"

#include "AIController.h"
#include "Interfaces/NpcInventoryInterface.h"

UBTTask_EquipBestWeapon::UBTTask_EquipBestWeapon()
{
	NodeName = "Equip best weapon";
}

EBTNodeResult::Type UBTTask_EquipBestWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Owner = Cast<INpcInventoryInterface>(OwnerComp.GetAIOwner()->GetPawn());
	if (Owner == nullptr)
		return EBTNodeResult::Failed;
	
	bool bSuccess = Owner->EquipBestWeapon_NPC();
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
