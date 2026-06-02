#include "BehaviorTree/Tasks/Actions/BTTask_UseHealingItem.h"

#include "AIController.h"
#include "Components/NpcHealingComponent.h"
#include "Data/AIGameplayTags.h"

UBTTask_UseHealingItem::UBTTask_UseHealingItem()
{
	NodeName = "Use best healing item";
}

void UBTTask_UseHealingItem::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_UseConsumable_Completed;
}

EBTNodeResult::Type UBTTask_UseHealingItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto HealingComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcHealingComponent>();
	if (!HealingComponent)
		return EBTNodeResult::Failed;
	
	Super::ExecuteTask(OwnerComp, NodeMemory);
	bool bStartedHealing = HealingComponent->Heal();
	if (!bStartedHealing)
		return EBTNodeResult::Failed;
	
	return bAwaitCompletion ? EBTNodeResult::InProgress : EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_UseHealingItem::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())
			if (auto HealingComponent = Pawn->FindComponentByClass<UNpcHealingComponent>())
				HealingComponent->AbortHeal();
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

FString UBTTask_UseHealingItem::GetStaticDescription() const
{
	return bAwaitCompletion ? FString::Printf(TEXT("Await completion\n%s"), *Super::GetStaticDescription()) : Super::GetStaticDescription();
}
