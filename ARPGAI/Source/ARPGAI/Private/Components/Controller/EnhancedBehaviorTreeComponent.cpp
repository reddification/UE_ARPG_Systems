// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Controller/EnhancedBehaviorTreeComponent.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/Composites/BTComposite_Utility.h"
#include "BehaviorTree/Tasks/BTTask_RunBehaviorDynamic.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcComponent.h"
#include "Data/AiDataTypes.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "VisualLogger/VisualLogger.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnhancedBehaviorTree, Log, Log)

void UEnhancedBehaviorTreeComponent::PauseLogic(const FString& Reason)
{
	if (IsPaused())
		return;
	
	ResetFlowControlBlackboardKeys();
	if (IsAbortPending())
	{
		UE_VLOG(GetOwner(), LogARPGAI, Log,
			TEXT("Pause requested when an abort is pending. For the sake of all holy on this planet, pause is postponed until no aborts are pending. Or shit may get fucked up."));
		bPausePending = true;		
	}
	else
	{
		if (auto NCL = GetNpcCombatLogicComponent(*this))
			NCL->SetBrainPaused(true);
		
		Super::PauseLogic(Reason);
	}
}

EAILogicResuming::Type UEnhancedBehaviorTreeComponent::ResumeLogic(const FString& Reason)
{
	auto Result = Super::ResumeLogic(Reason);
	ResetFlowControlBlackboardKeys();
	if (auto NCL = GetNpcCombatLogicComponent(*this))
		NCL->SetBrainPaused(false);
		
	// 01.02.2026 (aki)
	// this is a fix for situations when a latent task was aborted by a decorator,
	// but task refused to be aborted so it return InProgress and started waiting for brain message to finish latent abort
	// and after task has received brain message and called FinishLatentAbort, at the same frame (or in very close subsequent frames) PauseLogic was called
	// somehow this causes bRequestFlowUpdate to be set to false even though FinishLatentAbort sets it to true
	// so we're kickstarting BT component here in that case
	if (bRequestedFlowUpdate == false && (PendingExecution.IsSet() || ExecutionRequest.ExecuteNode != nullptr)) 
		ScheduleExecutionUpdate();
	
	if (bPausePending)
		bPausePending = false;
	
	return Result;
}

void UEnhancedBehaviorTreeComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (MessagesToProcess.Num() > 0 && TreeHasBeenStarted() && !IsPaused())
		ScheduleNextTick(0.0f);
	
	if (bPausePending)
	{
		if (IsPaused())
		{
			bPausePending = false;
			return;
		}
		
		if (!IsAbortPending())
		{
			bPausePending = false;
			Super::PauseLogic(TEXT("UEBALI"));
			if (auto NCL = GetNpcCombatLogicComponent(*this))
				NCL->SetBrainPaused(true);
		
		}
	}
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
		UE_VLOG(GetAIOwner(), LogARPGAI, VeryVerbose, TEXT("Handling non-immediate AI message %s"), *Message.MessageName.ToString());
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

	auto BehaviorTagsArray = BehaviorTags.GetGameplayTagArray();
	for (const auto& DynamicBehaviorTag : BehaviorTagsArray)
		if (auto BTPtr = BehaviorsConfiguration->DynamicBehaviors.Find(DynamicBehaviorTag))
			SetDynamicSubtree(DynamicBehaviorTag, *BTPtr, StartingNode);
}

void UEnhancedBehaviorTreeComponent::InitializeNpc(const FNpcDTR* NpcDTR)
{
	if (ensure(NpcDTR))
	{
		if (IsValid(NpcDTR->NpcBlackboardDataAsset))
			FlowControlBlackboardKeys = NpcDTR->NpcBlackboardDataAsset->FlowControlBlackboardKeys;
		
		if (!NpcDTR->BehaviorsConfiguration.IsNull())
			BehaviorsConfiguration = NpcDTR->BehaviorsConfiguration.LoadSynchronous();
	}
}

void UEnhancedBehaviorTreeComponent::AddStackedService(const FName& Key, UBTservice_ExclusiveStackedService* Service)
{
	auto& Stack = StackedServices.FindOrAdd(Key);
	if (!Stack.IsEmpty())
	{
		auto& PreviousService = Stack.Last();
		if (ensure(PreviousService.IsValid()))
		{
			auto* NodeMemory = GetNodeMemory(PreviousService.Get(), FindInstanceContainingNode(PreviousService.Get()));
			PreviousService->Freeze(NodeMemory);
		}
	}
	
	Stack.Add(Service);
}

void UEnhancedBehaviorTreeComponent::RemoveStackedService(const FName& Key, UBTservice_ExclusiveStackedService* Service)
{
	auto Stack = StackedServices.Find(Key);
	if (!Stack)
	{
		ensure(false);
		return;
	}
	
	const bool bWasLast = Stack->Last() == Service;
	Stack->Remove(Service);
	if (bWasLast && !Stack->IsEmpty())
	{
		const auto& NewTopSerivce = Stack->Last();
		if (ensure(NewTopSerivce.IsValid()))
		{
			auto* NodeMemory = GetNodeMemory(NewTopSerivce.Get(), FindInstanceContainingNode(NewTopSerivce.Get()));
			NewTopSerivce->Unfreeze(NodeMemory);
		}
	}
}

void UEnhancedBehaviorTreeComponent::AddUtilityObserver(const FBlackboardKeySelector& BBKey,
	const UBTComposite_Utility* UtilityComposite, int ChildIndex, bool bSuppressesRest)
{
	if (auto* ExistingContainer = UtilityBlackboardObservers.Find(BBKey.GetSelectedKeyID()))
	{
		ExistingContainer->Items.Add(FBTUtilityCompositeData (UtilityComposite, ChildIndex, bSuppressesRest));	
	}
	else
	{
		FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this,
			&UEnhancedBehaviorTreeComponent::OnUtilityChanged);
		auto& Container = UtilityBlackboardObservers.Add(BBKey.GetSelectedKeyID());
		Container.Items.Add(FBTUtilityCompositeData(UtilityComposite, ChildIndex, bSuppressesRest));
		Container.ObserverHandle = GetBlackboardComponent()->RegisterObserver(BBKey.GetSelectedKeyID(), this, ObserverDelegate);
	}
}

void UEnhancedBehaviorTreeComponent::RemoveUtilityObserver(const FBlackboardKeySelector& BBKey,
	const UBTComposite_Utility* UtilityComposite)
{
	auto* Container = UtilityBlackboardObservers.Find(BBKey.GetSelectedKeyID());
	if (!ensure(Container && !Container->Items.IsEmpty()))
		return;
	
	ensure(Container->Items.Last().Utility == UtilityComposite);
	for (int i = Container->Items.Num() - 1; i >= 0; --i)
	{
		if (Container->Items[i].Utility == UtilityComposite)
		{
			if (Container->Items[i].bSupressRest)
			{
				for (int j = i - 1; j >= 0; --j)
				{
					Container->Items[j].Utility->OnUtilityChanged(*this, BBKey.GetSelectedKeyID(), Container->Items[j].ChildIndex);
					if (Container->Items[j].bSupressRest)
						break;
				}
			}
			
			Container->Items.RemoveAt(i);
			break;
		}
	}
	
	if (Container->Items.IsEmpty())
	{
		GetBlackboardComponent()->UnregisterObserver(BBKey.GetSelectedKeyID(), Container->ObserverHandle);
		UtilityBlackboardObservers.Remove(BBKey.GetSelectedKeyID());
	}
}

void UEnhancedBehaviorTreeComponent::ResetFlowControlBlackboardKeys()
{
	if (FlowControlBlackboardKeys.IsEmpty())
		return;
	
	auto Blackboard = GetBlackboardComponent();
	for (const auto& FlowControlBB : FlowControlBlackboardKeys)
		Blackboard->ClearValue(FlowControlBB.SelectedKeyName);
}

EBlackboardNotificationResult UEnhancedBehaviorTreeComponent::OnUtilityChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	auto* Container = UtilityBlackboardObservers.Find(Key);
	if (Container == nullptr || Container->Items.IsEmpty())
		return EBlackboardNotificationResult::RemoveObserver;
	
	for (int i = Container->Items.Num() - 1; i >= 0; --i)
	{
		Container->Items[i].Utility->OnUtilityChanged(*this, Key, Container->Items[i].ChildIndex);
		if (Container->Items[i].bSupressRest)
			break;
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}
