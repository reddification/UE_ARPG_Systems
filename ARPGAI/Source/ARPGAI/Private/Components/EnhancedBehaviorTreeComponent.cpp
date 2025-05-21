// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/EnhancedBehaviorTreeComponent.h"

#include "AIController.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/Tasks/BTTask_RunBehaviorDynamic.h"
#include "Data/AiDataTypes.h"
#include "Data/LogChannels.h"
#include "VisualLogger/VisualLogger.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnhancedBehaviorTree, Log, Log)

bool SetDynamicSubtreeRecursive(const UBTCompositeNode* TestComposite,
	const FBehaviorTreeInstance& InstanceInfo, const UBehaviorTreeComponent* OwnerComp,
	const FGameplayTag& InjectTag, UBehaviorTree* BehaviorAsset)
{
	bool bInjected = false;

	for (int32 Idx = 0; Idx < TestComposite->Children.Num(); Idx++)
	{
		const FBTCompositeChild& ChildInfo = TestComposite->Children[Idx];
		if (ChildInfo.ChildComposite)
		{
			bInjected = (SetDynamicSubtreeRecursive(ChildInfo.ChildComposite, InstanceInfo, OwnerComp, InjectTag, BehaviorAsset) || bInjected);
		}
		else
		{
			UBTTask_RunBehaviorDynamic* SubtreeTask = Cast<UBTTask_RunBehaviorDynamic>(ChildInfo.ChildTask);
			if (SubtreeTask && SubtreeTask->HasMatchingTag(InjectTag))
			{
				const uint8* NodeMemory = SubtreeTask->GetNodeMemory<uint8>(InstanceInfo);
				UBTTask_RunBehaviorDynamic* InstancedNode = Cast<UBTTask_RunBehaviorDynamic>(SubtreeTask->GetNodeInstance(*OwnerComp, (uint8*)NodeMemory));
				if (InstancedNode)
				{
					const bool bAssetChanged = InstancedNode->SetBehaviorAsset(BehaviorAsset);
					if (bAssetChanged)
					{
						UE_VLOG(OwnerComp->GetOwner(), LogBehaviorTree, Log, TEXT("Replaced subtree in %s with %s (tag: %s)"),
							*UBehaviorTreeTypes::DescribeNodeHelper(SubtreeTask), *GetNameSafe(BehaviorAsset), *InjectTag.ToString());
						bInjected = true;
					}
				}
			}
		}
	}

	return bInjected;
}

void UEnhancedBehaviorTreeComponent::SetDynamicSubtree(FGameplayTag InjectTag, UBehaviorTree* BehaviorAsset)
{
	SetDynamicSubtreeAtIndex(InjectTag, BehaviorAsset, 0);
}

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

void UEnhancedBehaviorTreeComponent::SetDynamicSubtreeAtIndex(FGameplayTag InjectTag, UBehaviorTree* BehaviorAsset, const int32 InstanceStackStartIndex)
{
	// prevent infinite loops injection
	for (int32 i = 0; i < InstanceStackStartIndex; i++)
	{
		if (KnownInstances[InstanceStack[i].InstanceIdIndex].TreeAsset == BehaviorAsset)
		{
			UE_LOG(LogEnhancedBehaviorTree, Warning, TEXT("Attempt to inject a looping BT asset for [%s : %s]"), *InjectTag.ToString(), *GetNameSafe(BehaviorAsset))
			UE_LOG(LogEnhancedBehaviorTree, Warning, TEXT("It is already at stack %d [instance id = %d]"), i, InstanceStack[i].InstanceIdIndex)
			return;
		}
	}

	bool bInjected = false;
	for (int32 InstanceIndex = InstanceStackStartIndex; InstanceIndex < InstanceStack.Num(); InstanceIndex++)
	{
		const FBehaviorTreeInstance& InstanceInfo = InstanceStack[InstanceIndex];
		bInjected = (SetDynamicSubtreeRecursive(InstanceInfo.RootNode, InstanceInfo, this, InjectTag, BehaviorAsset) || bInjected);
	}

	// restart subtree if it was replaced
	if (bInjected)
	{
		for (int32 InstanceIndex = 0; InstanceIndex < InstanceStack.Num(); InstanceIndex++)
		{
			const FBehaviorTreeInstance& InstanceInfo = InstanceStack[InstanceIndex];
			if (InstanceInfo.ActiveNodeType == EBTActiveNode::ActiveTask)
			{
				const UBTTask_RunBehaviorDynamic* SubtreeTask = Cast<const UBTTask_RunBehaviorDynamic>(InstanceInfo.ActiveNode);
				if (SubtreeTask && SubtreeTask->HasMatchingTag(InjectTag))
				{
					UBTCompositeNode* RestartNode = SubtreeTask->GetParentNode();
					int32 RestartChildIdx = RestartNode->GetChildIndex(*SubtreeTask);

					RequestExecution(RestartNode, InstanceIndex, SubtreeTask, RestartChildIdx, EBTNodeResult::Aborted);
					break;
				}
			}
		}
	}
	else
	{
		UE_VLOG(GetOwner(), LogBehaviorTree, Log, TEXT("Failed to inject subtree %s at tag %s"), *GetNameSafe(BehaviorAsset), *InjectTag.ToString());
	}
}
