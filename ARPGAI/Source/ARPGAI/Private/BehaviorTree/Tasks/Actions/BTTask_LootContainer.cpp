#include "BehaviorTree/Tasks/Actions/BTTask_LootContainer.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcInventoryInterface.h"

UBTTask_LootContainer::UBTTask_LootContainer()
{
	NodeName = "Loot container";
	ContainerBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_LootContainer, ContainerBBKey), AActor::StaticClass());
	OutReactTagBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_LootContainer, OutReactTagBBKey)));
}

EBTNodeResult::Type UBTTask_LootContainer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto ContainerActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ContainerBBKey.SelectedKeyName));
	
	auto NpcInventoryInterface = Cast<INpcInventoryInterface>(OwnerComp.GetAIOwner()->GetPawn());
	if (NpcInventoryInterface == nullptr)
		return EBTNodeResult::Failed;
	
	Super::ExecuteTask(OwnerComp, NodeMemory);
	bool bStarted = NpcInventoryInterface->LootContainer_NPC(ContainerActor);
	if (!bStarted)
		return EBTNodeResult::Failed;
	
	for (const auto& Tag : LootCompletedAIMessages)
		WaitForMessage(OwnerComp, Tag.GetTagName());
	
	return EBTNodeResult::InProgress;
}

void UBTTask_LootContainer::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_LootContainer_Completed;
}

EBTNodeResult::Type UBTTask_LootContainer::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto NpcInventoryInterface = Cast<INpcInventoryInterface>(OwnerComp.GetAIOwner()->GetPawn());
	if (NpcInventoryInterface != nullptr)
		NpcInventoryInterface->StopLooting_NPC();
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_LootContainer::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message,
	int32 RequestID, bool bSuccess)
{
	const EBTTaskStatus::Type Status = OwnerComp.GetTaskStatus(this);
	if (Status == EBTTaskStatus::Active)
	{
		FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(Message, true);
		if (LootCompletedAIMessages.HasTag(MessageTag))
		{
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_GameplayTag>(OutReactTagBBKey.SelectedKeyName,
				MessageTag.GetSingleTagContainer());
		}
		else
		{
			UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Warning, TEXT("Received unknown message '%s' in UBTTask_LootContainer"), *Message.ToString());
		}
		
		FinishLatentTask(OwnerComp, bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
		return;
	}
	
	Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);
}

FString UBTTask_LootContainer::GetStaticDescription() const
{
	FString ListenedBrainMessageList;
	for (const auto& Tag : LootCompletedAIMessages)
		ListenedBrainMessageList += TEXT("\n\t") + Tag.ToString();	
	
	return FString::Printf(TEXT("Loot container %s\n[out]React to loot tag BB: %s\nAI messages:%s\n%s"),
		*ContainerBBKey.SelectedKeyName.ToString(), *OutReactTagBBKey.SelectedKeyName.ToString(),
		*ListenedBrainMessageList, *Super::GetStaticDescription());
}
