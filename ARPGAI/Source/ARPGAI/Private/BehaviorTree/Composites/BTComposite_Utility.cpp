
#include "BehaviorTree/Composites/BTComposite_Utility.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityBlackboard.h"
#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityFunction.h"
#include "Components/Controller/EnhancedBehaviorTreeComponent.h"
#include "Data/LogChannels.h"

UBTComposite_Utility::UBTComposite_Utility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Utility";
	bUseNodeActivationNotify = true;
	bUseNodeDeactivationNotify = true;
}

void UBTComposite_Utility::NotifyNodeActivation(FBehaviorTreeSearchData& SearchData) const
{
	UE_VLOG(SearchData.OwnerComp.GetAIOwner(), LogAI_Utility, Verbose, TEXT("UBTComposite_Utility:: Utility node activated: %s"), *GetNodeName());
	FBTUtilityMemory* NodeMemory = GetNodeMemory<FBTUtilityMemory>(SearchData);
	InitializeUtilityScores(SearchData, NodeMemory);
	if (auto* EnhancedBTComponent = Cast<UEnhancedBehaviorTreeComponent>(&SearchData.OwnerComp))
		RegisterUtilityObserver(EnhancedBTComponent, NodeMemory);
	else 
		WatchChildBlackboardKeys_Old(SearchData);
}

void UBTComposite_Utility::NotifyNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult) const
{
	UE_VLOG(SearchData.OwnerComp.GetAIOwner(), LogAI_Utility, Verbose, TEXT("UBTComposite_Utility:: Utility node DEactivated: %s"), *GetNodeName());
	FBTUtilityMemory* NodeMemory = GetNodeMemory<FBTUtilityMemory>(SearchData);
	if (auto* EnhancedBTComponent = Cast<UEnhancedBehaviorTreeComponent>(&SearchData.OwnerComp))
		UnregisterUtilityObserver(EnhancedBTComponent, NodeMemory);
	else 
		UnwatchChildBlackboardKeys_Old(SearchData);
}

const UBTDecorator_UtilityFunction* UBTComposite_Utility::FindChildUtilityDecorator(const int32 ChildIndex, UObject* LogOwner) const
{
	const FBTCompositeChild& ChildInfo = Children[ChildIndex];
	int DecoratorIndex = 0;
	UBTDecorator_UtilityFunction* UtilityDecorator = nullptr;
	
	for (; DecoratorIndex < ChildInfo.Decorators.Num(); ++DecoratorIndex)
	{
		UtilityDecorator = Cast<UBTDecorator_UtilityFunction>(Children[ChildIndex].Decorators[DecoratorIndex]);
		if (UtilityDecorator)
			break;
	}

	if (UtilityDecorator != nullptr && DecoratorIndex > 0)
	{
		UE_VLOG(LogOwner, LogAI_Utility, Warning,
			TEXT("Utility decorator is not the first decorator for child %d on %s. If an earlier decorator changes this child's utility during deactivation, Utility composite may request a redundant execution update."),
			ChildIndex, *GetName());
	}
	
	return UtilityDecorator;
}

void UBTComposite_Utility::InitializeUtilityScores(FBehaviorTreeSearchData& SearchData, FBTUtilityMemory* UtilityMemory) const
{
	uint8 Count = 0;
	auto LogOwner = SearchData.OwnerComp.GetAIOwner();
	for(int32 i = 0; i < GetChildrenNum(); ++i)
	{
		if (Count >= MAX_UTILITY_NODES)
		{
			UE_VLOG(LogOwner, LogAI_Utility, Warning, TEXT("[%s] BT utility composite contains more than %d utility children. Remaining won't get evaluated"), 
				*GetTreeAsset()->GetName(), MAX_UTILITY_NODES);
			break;
		}
		
		if (const UBTDecorator_UtilityFunction* UtilityDecorator = FindChildUtilityDecorator(i, LogOwner))
		{
			const float Score = UtilityDecorator
			   ? UtilityDecorator->WrappedCalculateUtility(SearchData.OwnerComp, UtilityDecorator->GetNodeMemory<uint8>(SearchData))
			   : 0.0f;

			UtilityMemory->ExecutionUtilityOrdering[Count].ChildIdx = i;
			UtilityMemory->ExecutionUtilityOrdering[Count].UtilityScore = Score;
			UtilityMemory->ExecutionUtilityOrdering[Count].BaseUtilityDecorator = UtilityDecorator;
			if (auto BlackboardUtility = Cast<UBTDecorator_UtilityBlackboard>(UtilityDecorator))
				UtilityMemory->ExecutionUtilityOrdering[Count].BlackboardUtilityDecorator = BlackboardUtility;
			else if (UtilityMemory->ExecutionUtilityOrdering[Count].BlackboardUtilityDecorator.IsValid())
				UtilityMemory->ExecutionUtilityOrdering[Count].BlackboardUtilityDecorator.Reset(); // ху блять нихуя себе 
			
			Count++;
		}
		else
		{
			UE_VLOG(LogOwner, LogAI_Utility, Warning, 
				TEXT("[%s] BT utility composite child %d doesn't have a Utility decorator. This branch will never be executed"),
				*GetTreeAsset()->GetName(), i);
		}
	}

	UtilityMemory->ActualUtilityNodesCount = Count;
	for (uint8 i = Count; i < MAX_UTILITY_NODES; i++)
		UtilityMemory->ExecutionUtilityOrdering[i].SetInvalid();

	Algo::StableSort(UtilityMemory->ExecutionUtilityOrdering);
	
#if WITH_EDITOR
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("Utilities after EvaluateUtilityScores:"));
	for (int i = 0; i < UtilityMemory->ActualUtilityNodesCount; i++)
	{
		UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("[%d]: %s %.2f (actual child index = %d)"),
			i, *UtilityMemory->ExecutionUtilityOrdering[i].GetBlackboardKeyName().ToString(),
			UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore, UtilityMemory->ExecutionUtilityOrdering[i].ChildIdx);
	}
	
#endif
}

void UBTComposite_Utility::RegisterUtilityObserver(UEnhancedBehaviorTreeComponent* EnhancedBTComponent, const FBTUtilityMemory* BTMemory) const
{
	for (int i = 0; i < BTMemory->ActualUtilityNodesCount; i++)
	{
		if (BTMemory->ExecutionUtilityOrdering[i].BlackboardUtilityDecorator == nullptr)
			continue;
		
		EnhancedBTComponent->AddUtilityObserver(BTMemory->ExecutionUtilityOrdering[i].BlackboardUtilityDecorator->GetSelectedBlackboardKeySelector(),
			this, BTMemory->ExecutionUtilityOrdering[i].ChildIdx, 
			BTMemory->ExecutionUtilityOrdering[i].BlackboardUtilityDecorator->IsSupressesPrevious());
	}
}

void UBTComposite_Utility::UnregisterUtilityObserver(UEnhancedBehaviorTreeComponent* EnhancedBTComponent, const FBTUtilityMemory* BTMemory) const
{
	for (int i = 0; i < BTMemory->ActualUtilityNodesCount; i++)
	{
		if (BTMemory->ExecutionUtilityOrdering[i].BlackboardUtilityDecorator == nullptr)
			continue;
		
		EnhancedBTComponent->RemoveUtilityObserver(BTMemory->ExecutionUtilityOrdering[i].BlackboardUtilityDecorator->GetSelectedBlackboardKeySelector(),
			this);
	}
}

void UBTComposite_Utility::WatchChildBlackboardKeys_Old(FBehaviorTreeSearchData& SearchData) const
{
	UE_VLOG(SearchData.OwnerComp.GetAIOwner(), LogAI_Utility, Verbose, TEXT("UBTComposite_Utility::WatchChildBlackboardKeys [%s]"), *GetNodeName());
	UBlackboardComponent* BlackboardComp = SearchData.OwnerComp.GetBlackboardComponent();
	if (!ensure(BlackboardComp))
		return;

	auto MutableThis = const_cast<UBTComposite_Utility*>(this);
	for (const FBTCompositeChild& Child : Children)
	{
		for (UBTDecorator* Decorator : Child.Decorators)
		{
			if (UBTDecorator_UtilityBlackboard* BlackboardUtilityDecorator = Cast<UBTDecorator_UtilityBlackboard>(Decorator))
			{
				const auto& SelectedBlackboardKeySelector = BlackboardUtilityDecorator->GetSelectedBlackboardKeySelector();
				FBlackboard::FKey KeyID = SelectedBlackboardKeySelector.GetSelectedKeyID();
				FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(MutableThis,
					&UBTComposite_Utility::OnUtilityChanged_Old, BlackboardUtilityDecorator->GetChildIndex());
				BlackboardComp->RegisterObserver(KeyID, BlackboardUtilityDecorator, ObserverDelegate);

				break;
			}
		}
	}
}

void UBTComposite_Utility::UnwatchChildBlackboardKeys_Old(FBehaviorTreeSearchData& SearchData) const
{
	UE_VLOG(SearchData.OwnerComp.GetAIOwner(), LogAI_Utility, Verbose, TEXT("UBTComposite_Utility::UnwatchChildBlackboardKeys [%s]"), *GetNodeName());
	for (const FBTCompositeChild& Child : Children)
	{
		for (UBTDecorator* Decorator : Child.Decorators)
		{
			if (UBTDecorator_UtilityBlackboard* BlackboardUtilityDecorator = Cast<UBTDecorator_UtilityBlackboard>(Decorator))
			{
				if (UBlackboardComponent* BlackboardComp = SearchData.OwnerComp.GetBlackboardComponent())
				{
					FBlackboardKeySelector SelectedBlackboardKeySelector = BlackboardUtilityDecorator->GetSelectedBlackboardKeySelector();
					BlackboardComp->UnregisterObserversFrom(BlackboardUtilityDecorator);
				}

				break;
			}
		}
	}
}

void UBTComposite_Utility::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	InitializeNodeMemory<FBTUtilityMemory>(NodeMemory, InitType);
}

void UBTComposite_Utility::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	CleanupNodeMemory<FBTUtilityMemory>(NodeMemory, CleanupType);
}

EBlackboardNotificationResult UBTComposite_Utility::OnUtilityChanged_Old(const UBlackboardComponent& Blackboard,
	FBlackboard::FKey Key, uint8 ChildIndex)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	uint8* BTUtilityMemoryPtr = BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(this));
	if (!ensure(BTUtilityMemoryPtr))
		return EBlackboardNotificationResult::RemoveObserver;
		
	FBTUtilityMemory* BTUtilityMemory = reinterpret_cast<FBTUtilityMemory*>(BTUtilityMemoryPtr);
	if (BTUtilityMemory->ActualUtilityNodesCount < 2)
		return EBlackboardNotificationResult::RemoveObserver;

	float NewUtilityValue = Blackboard.GetValue<UBlackboardKeyType_Float>(Key);
	auto AIController = BehaviorComp->GetAIOwner();
	
	UE_VLOG(AIController, LogAI_Utility, VeryVerbose, TEXT("Utility changed for %s. Child index = %d. New utility value = %.2f"),
		*Blackboard.GetKeyName(Key).ToString(), ChildIndex, NewUtilityValue);

	// TArray<FIndexedUtilityValue> OldExecutionOrder(BTUtilityMemory->ExecutionUtilityOrdering, BTUtilityMemory->ActualUtilityNodesCount);
	
	FIndexedUtilityValue OldExecutionOrder[MAX_UTILITY_NODES] = {};
	FMemory::Memcpy(OldExecutionOrder, BTUtilityMemory->ExecutionUtilityOrdering, sizeof(BTUtilityMemory->ExecutionUtilityOrdering));
	
	for (auto i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
	{
		if (BTUtilityMemory->ExecutionUtilityOrdering[i].ChildIdx == ChildIndex)
		{
			BTUtilityMemory->ExecutionUtilityOrdering[i].UtilityScore = NewUtilityValue;
			break;
		}
	}

	Algo::StableSort(BTUtilityMemory->ExecutionUtilityOrdering);
	if (OldExecutionOrder[0].ChildIdx != BTUtilityMemory->ExecutionUtilityOrdering[0].ChildIdx)
	{
		
#if WITH_EDITOR
		UE_VLOG(AIController, LogAI_Utility, Verbose, TEXT("Utility order changed:"));
		UE_VLOG(AIController, LogAI_Utility, Verbose, TEXT("=====================:"));
		for (int i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
		{
			UE_VLOG(AIController, LogAI_Utility, Verbose,
				TEXT("OldExecutionOrder[%d]: Utility = %.2f, child index = %d"), i, OldExecutionOrder[i].UtilityScore, OldExecutionOrder[i].ChildIdx);
		}

		UE_VLOG(AIController, LogAI_Utility, Verbose, TEXT("=====================:"));
		for (int i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
		{
			UE_VLOG(AIController, LogAI_Utility, Verbose,
				TEXT("NewExecutionOrder[%d]: Utility = %.2f, child index = %d"), i, BTUtilityMemory->ExecutionUtilityOrdering[i].UtilityScore, BTUtilityMemory->ExecutionUtilityOrdering[i].ChildIdx);
		}
		
		UE_VLOG(AIController, LogAI_Utility, Verbose, TEXT("=====================:"));
#endif
		
		EvaluateUtilityExecution(*BehaviorComp, BTUtilityMemory, ChildIndex);
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTComposite_Utility::OnUtilityChanged(UBehaviorTreeComponent& BehaviorComp, FBlackboard::FKey Key, uint8 ChildIndex) const
{
	uint8* BTUtilityMemoryPtr = BehaviorComp.GetNodeMemory(this, BehaviorComp.FindInstanceContainingNode(this));
	if (!ensure(BTUtilityMemoryPtr))
		return;
		
	FBTUtilityMemory* BTUtilityMemory = reinterpret_cast<FBTUtilityMemory*>(BTUtilityMemoryPtr);
	if (BTUtilityMemory->ActualUtilityNodesCount < 2)
		return;

	auto Blackboard = BehaviorComp.GetBlackboardComponent();
	const float NewUtilityValue = Blackboard->GetValue<UBlackboardKeyType_Float>(Key);
	
	auto AIController = BehaviorComp.GetAIOwner();
	UE_VLOG(AIController, LogAI_Utility, VeryVerbose, TEXT("Utility changed for %s. Child index = %d. New utility value = %.2f"),
		*Blackboard->GetKeyName(Key).ToString(), ChildIndex, NewUtilityValue);

	// TArray<FIndexedUtilityValue> OldExecutionOrder(BTUtilityMemory->ExecutionUtilityOrdering, BTUtilityMemory->ActualUtilityNodesCount);
	
	FIndexedUtilityValue OldExecutionOrder[MAX_UTILITY_NODES] = {};
	FMemory::Memcpy(OldExecutionOrder, BTUtilityMemory->ExecutionUtilityOrdering, sizeof(BTUtilityMemory->ExecutionUtilityOrdering));
	
	for (auto i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
	{
		if (BTUtilityMemory->ExecutionUtilityOrdering[i].ChildIdx == ChildIndex)
		{
			BTUtilityMemory->ExecutionUtilityOrdering[i].UtilityScore = NewUtilityValue;
			break;
		}
	}

	Algo::StableSort(BTUtilityMemory->ExecutionUtilityOrdering);
	if (OldExecutionOrder[0].ChildIdx != BTUtilityMemory->ExecutionUtilityOrdering[0].ChildIdx)
	{
		
#if WITH_EDITOR
		UE_VLOG(AIController, LogAI_Utility, Verbose, TEXT("Utility order changed:"));
		UE_VLOG(AIController, LogAI_Utility, Verbose, TEXT("=====================:"));
		for (int i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
		{
			UE_VLOG(AIController, LogAI_Utility, Verbose,
				TEXT("OldExecutionOrder[%d]: Utility = %.2f, [%s] child index = %d"), i, OldExecutionOrder[i].UtilityScore, 
					*OldExecutionOrder[i].GetBlackboardKeyName().ToString(), OldExecutionOrder[i].ChildIdx);
		}

		UE_VLOG(AIController, LogAI_Utility, Verbose, TEXT("=====================:"));
		for (int i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
		{
			UE_VLOG(AIController, LogAI_Utility, Verbose,
				TEXT("NewExecutionOrder[%d]: Utility = %.2f, [%s] child index = %d"), i, BTUtilityMemory->ExecutionUtilityOrdering[i].UtilityScore,
				*BTUtilityMemory->ExecutionUtilityOrdering[i].GetBlackboardKeyName().ToString(), BTUtilityMemory->ExecutionUtilityOrdering[i].ChildIdx);
		}
		
		UE_VLOG(AIController, LogAI_Utility, Verbose, TEXT("=====================:"));
#endif
		
		EvaluateUtilityExecution(BehaviorComp, BTUtilityMemory, ChildIndex);
	}
}

void UBTComposite_Utility::EvaluateUtilityExecution(UBehaviorTreeComponent& BehaviorComp, FBTUtilityMemory* BTUtilityMemory, uint8 ChildIndex) const
{
	BTUtilityMemory->bUtilityReevaluated = true;
	const bool bChangedForCurrentlyExecutingChild = ChildIndex == BTUtilityMemory->CurrentChild;// BehaviorComp.IsExecutingBranch(this, ChildIndex);
	const UBTDecorator_UtilityFunction* UtilityDecorator = nullptr;
	for (int i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
	{
		if (BTUtilityMemory->ExecutionUtilityOrdering[i].ChildIdx == ChildIndex)
		{
			if (BTUtilityMemory->ExecutionUtilityOrdering[i].BaseUtilityDecorator.IsValid())
				UtilityDecorator = BTUtilityMemory->ExecutionUtilityOrdering[i].BaseUtilityDecorator.Get();
			
			break;
		}
	}
		
	const bool bUtilityChildAlreadyDeactivating = UtilityDecorator && !UtilityDecorator->IsBranchActive(BehaviorComp);
	const bool bMustRequestExecution = !(bChangedForCurrentlyExecutingChild && bUtilityChildAlreadyDeactivating);
	if (bMustRequestExecution && ensure(UtilityDecorator))
		BehaviorComp.RequestExecution(UtilityDecorator);
}

void UBTComposite_Utility::UpdateUtilityScores(FBTUtilityMemory* UtilityMemory, const UBlackboardComponent* Blackboard) const
{
	FIndexedUtilityValue Previous[MAX_UTILITY_NODES]; 
	FMemory::Memcpy(Previous, UtilityMemory->ExecutionUtilityOrdering, sizeof(Previous));
	
	auto LogOwner = Blackboard->GetBrainComponent()->GetAIOwner();
	for (int i = 0; i < UtilityMemory->ActualUtilityNodesCount; i++)
	{
		const auto& BBKey = UtilityMemory->ExecutionUtilityOrdering[i].GetBlackboardKey();
		if (BBKey != FBlackboard::InvalidKey)
		{
			float OldValue = UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore;
			UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore = Blackboard->GetValue<UBlackboardKeyType_Float>(BBKey);
#if WITH_EDITOR
			if (OldValue != UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore)
			{
				UE_VLOG(LogOwner, LogAI_Utility, Warning,
					TEXT("UBTComposite_Utility::UpdateUtilityScores: Utility mismatch between Node Memory and actual blackboard data for utility %s [child %d]. [NM] %.2f != [BB] %.2f"),
					*UtilityMemory->ExecutionUtilityOrdering[i].GetBlackboardKeyName().ToString(), UtilityMemory->ExecutionUtilityOrdering[i].ChildIdx,
					OldValue, UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore);
			}
#endif
		}
	}
	
	Algo::StableSort(UtilityMemory->ExecutionUtilityOrdering);
	
	for (int i = 0; i < UtilityMemory->ActualUtilityNodesCount; i++)
	{
		if (Previous[i].ChildIdx != UtilityMemory->ExecutionUtilityOrdering[i].ChildIdx)
		{
			UE_VLOG(LogOwner, LogAI_Utility, Warning,
				TEXT("UBTComposite_Utility::UpdateUtilityScores: Child order mismatch. At [%d] old child idx = %d; new child idx = %d"),
				i, Previous[i].ChildIdx, UtilityMemory->ExecutionUtilityOrdering[i].ChildIdx);
			
			UtilityMemory->bUtilityReevaluated = true;
		}
	}
}

int32 UBTComposite_Utility::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	if (LastResult == EBTNodeResult::Aborted)
		return Super::GetNextChildHandler(SearchData, PrevChild, LastResult);
	
	FBTUtilityMemory* UtilityMemory = GetNodeMemory<FBTUtilityMemory>(SearchData);
	if (UtilityMemory->ActualUtilityNodesCount == 0)
	{
		UE_VLOG(SearchData.OwnerComp.GetAIOwner(), LogAI_Utility, Warning,
			TEXT("UBTComposite_Utility::GetNextChildHandler: no valid utility children for %s"),
			*GetNodeName());

		return BTSpecialChild::ReturnToParent;
	}
	
	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	int32 CurrentUtilityItemIndex = 0;
	if (PrevChild != BTSpecialChild::NotInitialized)	
	{
		UpdateUtilityScores(UtilityMemory, Blackboard);
		for (uint8 i = 0; i < UtilityMemory->ActualUtilityNodesCount; ++i)
		{
			if (UtilityMemory->ExecutionUtilityOrdering[i].ChildIdx == PrevChild)
			{
				CurrentUtilityItemIndex = i;
				break;
			}
		}
	}
	
#if WITH_EDITOR
	auto LogOwner = SearchData.OwnerComp.GetAIOwner();
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("UBTComposite_Utility::GetNextChildHandler"));
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("Previous child = %d"), PrevChild);
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("Last result = %s"), *UBehaviorTreeTypes::DescribeNodeResult(LastResult));
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("On success: %s"), *StaticEnum<EUtilityNodeCompletedBehavior>()->GetDisplayValueAsText(OnSuccessBehavior).ToString());
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("On failure: %s"), *StaticEnum<EUtilityNodeCompletedBehavior>()->GetDisplayValueAsText(OnFailureBehavior).ToString());
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("Iterator index = %d"), CurrentUtilityItemIndex);
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("Utilities:"));
	
	for (int i = 0; i < UtilityMemory->ActualUtilityNodesCount; i++)
	{
		if (!UtilityMemory->ExecutionUtilityOrdering[i].GetBlackboardKeyName().IsNone())
		{
			const float ActualBlackboardValue = Blackboard->GetValueAsFloat(UtilityMemory->ExecutionUtilityOrdering[i].GetBlackboardKeyName());
			if (!ensure(FMath::IsNearlyEqual(ActualBlackboardValue, UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore)))
			{
				UE_VLOG(LogOwner, LogAI_Utility, Error, TEXT("UBTComposite_Utility::GetNextChildHandler: SHIT'S FUCKED UP! Utility %s mismatch: %.2f [NM] != %.2f [BB]"),
					*UtilityMemory->ExecutionUtilityOrdering[i].GetBlackboardKeyName().ToString(), UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore, ActualBlackboardValue);
			}
		}
		
		UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("[%d]: %s %.2f (actual child index = %d)"),
			i, *UtilityMemory->ExecutionUtilityOrdering[i].GetBlackboardKeyName().ToString(),
			UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore, UtilityMemory->ExecutionUtilityOrdering[i].ChildIdx);
	}
#endif
	
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;

	if (PrevChild == BTSpecialChild::NotInitialized || UtilityMemory->bUtilityReevaluated)
	{
		NextChildIdx = UtilityMemory->ExecutionUtilityOrdering[0].ChildIdx;
		CurrentUtilityItemIndex = 0;
	}
	else
	{
		EUtilityNodeCompletedBehavior UtilityNodeCompletedBehavior = EUtilityNodeCompletedBehavior::Finish; 
		if (LastResult == EBTNodeResult::Succeeded)
			UtilityNodeCompletedBehavior = OnSuccessBehavior;
		else if (LastResult == EBTNodeResult::Failed)
			UtilityNodeCompletedBehavior = OnFailureBehavior;
		
		if (UtilityNodeCompletedBehavior == EUtilityNodeCompletedBehavior::Next)
		{
			if (CurrentUtilityItemIndex + 1 < UtilityMemory->ActualUtilityNodesCount)
			{
				NextChildIdx = UtilityMemory->ExecutionUtilityOrdering[CurrentUtilityItemIndex + 1].ChildIdx;
			}
		}
		else if (UtilityNodeCompletedBehavior == EUtilityNodeCompletedBehavior::Repeat)
		{
			NextChildIdx = UtilityMemory->ExecutionUtilityOrdering[CurrentUtilityItemIndex].ChildIdx;
		}
	}
	
	
	UtilityMemory->bUtilityReevaluated = false;
#if WITH_EDITOR
	UE_VLOG(LogOwner, LogAI_Utility, VeryVerbose, TEXT("Next child = %d"), NextChildIdx);
#endif
	return NextChildIdx;
}

uint16 UBTComposite_Utility::GetInstanceMemorySize() const
{
	return sizeof(FBTUtilityMemory);
}

FString UBTComposite_Utility::GetStaticDescription() const
{
	return TEXT("Select child node with the highest score");
}

FBlackboard::FKey FIndexedUtilityValue::GetBlackboardKey() const
{
	return BlackboardUtilityDecorator != nullptr ? BlackboardUtilityDecorator->GetSelectedBlackboardKeySelector().GetSelectedKeyID() : FBlackboard::InvalidKey;
}

FName FIndexedUtilityValue::GetBlackboardKeyName() const
{
	return BlackboardUtilityDecorator != nullptr ? BlackboardUtilityDecorator->GetSelectedBlackboardKeyName() : NAME_None;
}

void FIndexedUtilityValue::SetInvalid()
{
	ChildIdx = -1;
	UtilityScore = -FLT_MAX;
	BaseUtilityDecorator = nullptr;
	BlackboardUtilityDecorator = nullptr;
}
