// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Controller/EnhancedBehaviorTreeComponent.h"

#include "AIController.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/Tasks/BTTask_RunBehaviorDynamic.h"
#include "Components/NpcComponent.h"
#include "Data/AiDataTypes.h"
#include "Data/LogChannels.h"
#include "VisualLogger/VisualLogger.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnhancedBehaviorTree, Log, Log)

void UEnhancedBehaviorTreeComponent::HandleMessage(const FAIMessage& Message)
{
	if (Message.HasFlag(AI_BRAINMESSAGE_FLAG_IMMEDIATE))
	{
		UE_VLOG(GetAIOwner(), LogARPGAI, Verbose, TEXT("Handling immediate AI message %s"), *Message.MessageName.ToString());
		HandleMessageImmediately(Message);
	}
	else
	{
		UE_VLOG(GetAIOwner(), LogARPGAI, Verbose, TEXT("Handling non-immediate AI message %s"), *Message.MessageName.ToString());
		Super::HandleMessage(Message);
	}
}

void UEnhancedBehaviorTreeComponent::HandleMessageImmediately(const FAIMessage& Message)
{
	for (int32 ObserverIndex = 0; ObserverIndex < MessageObservers.Num(); ObserverIndex++)
	{
		if (MessageObservers[ObserverIndex]->GetObservedMessageType() == Message.MessageName)
		{
			MessageObservers[ObserverIndex]->OnMessage(Message);
			break;
		}
	}
}

void UEnhancedBehaviorTreeComponent::LoadDynamicTrees(const FGameplayTagContainer& BehaviorTags,
	UBTCompositeNode* StartingNode)
{
	if (BehaviorTags.IsEmpty())
		return;

	auto NpcComponent = GetAIOwner()->GetPawn()->FindComponentByClass<UNpcComponent>();
	auto NpcDTR = NpcComponent->GetNpcDTR();
	auto BehaviorTagsArray = BehaviorTags.GetGameplayTagArray();
	
	for (const auto& DynamicBehaviorTag : BehaviorTagsArray)
	{
		if (auto BTPtr = NpcDTR->DynamicBehaviors.Find(DynamicBehaviorTag))
		{
			auto BT = const_cast<UBehaviorTree*>(*BTPtr);
			SetDynamicSubtree(DynamicBehaviorTag, BT, StartingNode);
		}
	}
}
