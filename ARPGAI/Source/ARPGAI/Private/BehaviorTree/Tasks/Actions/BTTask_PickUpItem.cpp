#include "BehaviorTree/Tasks/Actions/BTTask_PickUpItem.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcInventoryInterface.h"
#include "Interfaces/NpcValueableItemInterface.h"

UBTTask_PickUpItem::UBTTask_PickUpItem()
{
	NodeName = "Pick up item";
	ItemBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PickUpItem, ItemBBKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_PickUpItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Item = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ItemBBKey.SelectedKeyName));
	if (Item == nullptr)
		return EBTNodeResult::Failed;
	
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	
	auto ValueableItemInterface = Cast<INpcValueableItemInterface>(Item);
	if (ValueableItemInterface != nullptr)
		if (!ValueableItemInterface->CanPickUp_NPC(Pawn))
			return EBTNodeResult::Failed;
	
	auto Npc = Cast<INpc>(Pawn);
	if (Npc == nullptr)
		return EBTNodeResult::Failed;
	
	Super::ExecuteTask(OwnerComp, NodeMemory);
	const bool bStarted = Npc->PickUpItem_NPC(Item);
	return bStarted ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

FString UBTTask_PickUpItem::GetStaticDescription() const
{
	return FString::Printf(TEXT("Pick up %s\n%s"), *ItemBBKey.SelectedKeyName.ToString(), *Super::GetStaticDescription());
}

void UBTTask_PickUpItem::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_PickUp_Completed;
}
