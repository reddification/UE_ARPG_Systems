
#include "BehaviorTree/Composites/BTComposite_Utility.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityBlackboard.h"
#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityFunction.h"
#include "Data/LogChannels.h"

UBTComposite_Utility::UBTComposite_Utility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Utility";
	bUseNodeActivationNotify = true;
	bUseNodeDeactivationNotify = true;
}

void UBTComposite_Utility::NotifyNodeActivation(FBehaviorTreeSearchData& SearchData) const
{
	OrderUtilityScores(SearchData);
	WatchChildBlackboardKeys(SearchData);
}

void UBTComposite_Utility::NotifyNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult) const
{
	UnwatchChildBlackboardKeys(SearchData);
}

const UBTDecorator_UtilityFunction* UBTComposite_Utility::FindChildUtilityDecorator(int32 ChildIndex) const
{
	const FBTCompositeChild& ChildInfo = Children[ChildIndex];
	for (const TObjectPtr<UBTDecorator> Decorator : ChildInfo.Decorators)
	{
		if (UBTDecorator_UtilityFunction* UtilityDecorator = Cast<UBTDecorator_UtilityFunction>(Decorator))
		{
			return UtilityDecorator;
		}
	}

	return nullptr;
}

void UBTComposite_Utility::OrderUtilityScores(FBehaviorTreeSearchData& SearchData) const
{
	FBTUtilityMemory* NodeMemory = GetNodeMemory<FBTUtilityMemory>(SearchData);
	EvaluateUtilityScores(SearchData, NodeMemory);
}

void UBTComposite_Utility::EvaluateUtilityScores(FBehaviorTreeSearchData& SearchData, FBTUtilityMemory* UtilityMemory) const
{
	uint8 Count = 0;
	for(int32 i = 0; i < GetChildrenNum(); ++i)
	{
		if (Count > MAX_UTILITY_NODES)
		{
			UE_LOG(LogAIUtility, Warning, TEXT("[%s] BT utility composite contains more than %d utility children. Remaining won't get evaluated"), *GetTreeAsset()->GetName(), MAX_UTILITY_NODES)
			UtilityMemory->ExecutionUtilityOrdering[i].SetInvalid();
			break;
		}
		
		if (const UBTDecorator_UtilityFunction* UtilityDecorator = FindChildUtilityDecorator(i))
		{
			const float Score = UtilityDecorator
			   ? UtilityDecorator->WrappedCalculateUtility(SearchData.OwnerComp, UtilityDecorator->GetNodeMemory<uint8>(SearchData))
			   : 0.0f;

			UtilityMemory->ExecutionUtilityOrdering[i].ChildIdx = i;
			UtilityMemory->ExecutionUtilityOrdering[i].UtilityScore = Score;
			Count++;
		}
		else
		{
			UE_LOG(LogAIUtility, Warning, TEXT("[%s] BT utility composite contains node without Utility decorator. This node won't get executed"), *GetTreeAsset()->GetName())
		}
	}

	UtilityMemory->ActualUtilityNodesCount = Count;
	for (uint8 i = Count; i < MAX_UTILITY_NODES; i++)
	{
		UtilityMemory->ExecutionUtilityOrdering[i].SetInvalid();
	}

	Algo::StableSort(UtilityMemory->ExecutionUtilityOrdering);
}

void UBTComposite_Utility::WatchChildBlackboardKeys(FBehaviorTreeSearchData& SearchData) const
{
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
				FBlackboardKeySelector SelectedBlackboardKeySelector = BlackboardUtilityDecorator->GetSelectedBlackboardKeySelector();
				FBlackboard::FKey KeyID = SelectedBlackboardKeySelector.GetSelectedKeyID();
				FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(MutableThis,
					&UBTComposite_Utility::OnUtilityChanged, BlackboardUtilityDecorator->GetChildIndex());
				BlackboardComp->RegisterObserver(KeyID, BlackboardUtilityDecorator, ObserverDelegate);

				break;
			}
		}
	}
}

void UBTComposite_Utility::UnwatchChildBlackboardKeys(FBehaviorTreeSearchData& SearchData) const
{
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

// @AK 12.09.2024 fucking UE 5.4 gone insane and gives some obscure errors which are resolved by overriding some virtual methods
void UBTComposite_Utility::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	// Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	InitializeNodeMemory<FBTCompositeMemory>(NodeMemory, InitType);
}

void UBTComposite_Utility::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	// Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
	CleanupNodeMemory<FBTCompositeMemory>(NodeMemory, CleanupType);
}

EBlackboardNotificationResult UBTComposite_Utility::OnUtilityChanged(const UBlackboardComponent& Blackboard,
	FBlackboard::FKey Key, uint8 ChildIndex)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	uint8* BTUtilityMemoryPtr = BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(GetParentNode()));
	if (!ensure(BTUtilityMemoryPtr))
		return EBlackboardNotificationResult::RemoveObserver;
		
	FBTUtilityMemory* BTUtilityMemory = reinterpret_cast<FBTUtilityMemory*>(BTUtilityMemoryPtr);
	if (BTUtilityMemory->ActualUtilityNodesCount < 2)
		return EBlackboardNotificationResult::RemoveObserver;

	float NewUtilityValue = Blackboard.GetValue<UBlackboardKeyType_Float>(Key);
	auto AIController = BehaviorComp->GetAIOwner();
	
	UE_VLOG(AIController, LogARPGAI_Utility, VeryVerbose, TEXT("Utility changed for %s. Child index = %d. New utility value = %.2f"),
		*Blackboard.GetKeyName(Key).ToString(), ChildIndex, NewUtilityValue);

	// TArray<FIndexedUtilityValue> OldExecutionOrder(BTUtilityMemory->ExecutionUtilityOrdering, BTUtilityMemory->ActualUtilityNodesCount);
	
	FIndexedUtilityValue OldExecutionOrder[MAX_UTILITY_NODES] = {};
	memcpy(OldExecutionOrder, BTUtilityMemory->ExecutionUtilityOrdering, sizeof(BTUtilityMemory->ExecutionUtilityOrdering));
	
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
		UE_VLOG(AIController, LogARPGAI_Utility, Verbose, TEXT("Utility order changed:"));
		for (int i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
		{
			UE_VLOG(AIController, LogARPGAI_Utility, Verbose,
				TEXT("OldExecutionOrder[%d]: Utility = %.2f, child index = %d"), i, OldExecutionOrder[i].UtilityScore, OldExecutionOrder[i].ChildIdx);
		}

		for (int i = 0; i < BTUtilityMemory->ActualUtilityNodesCount; i++)
		{
			UE_VLOG(AIController, LogARPGAI_Utility, Verbose,
				TEXT("NewExecutionOrder[%d]: Utility = %.2f, child index = %d"), i, BTUtilityMemory->ExecutionUtilityOrdering[i].UtilityScore, BTUtilityMemory->ExecutionUtilityOrdering[i].ChildIdx);
		}
#endif
		
		BTUtilityMemory->bUtilityChanged = true;
		for (auto& Decorator : Children[ChildIndex].Decorators)
		{
			if (auto UtilityDecorator = Cast<UBTDecorator_UtilityFunction>(Decorator))
			{
				BehaviorComp->RequestExecution(UtilityDecorator);
				return EBlackboardNotificationResult::ContinueObserving;
			}
		}
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

int32 UBTComposite_Utility::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	FBTUtilityMemory* NodeMemory = GetNodeMemory<FBTUtilityMemory>(SearchData);
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;

	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		NextChildIdx = NodeMemory->ExecutionUtilityOrdering[0].ChildIdx;
	}
	else if (((LastResult == EBTNodeResult::Aborted || LastResult == EBTNodeResult::Failed) && NodeMemory->bUtilityChanged)
			|| (LastResult == EBTNodeResult::Succeeded && bReevaluateOnBranchSucceeded))
	{
		NextChildIdx = NodeMemory->ExecutionUtilityOrdering[0].ChildIdx;	
	}
	
	NodeMemory->bUtilityChanged = false;
	return NextChildIdx;
}

void UBTComposite_Utility::Reevaluate(UBehaviorTreeComponent* BehaviorTreeComponent)
{
	for (const FBTCompositeChild& Child : Children)
	{
		for (UBTDecorator* Decorator : Child.Decorators)
		{
			if (UBTDecorator_UtilityBlackboard* BlackboardUtilityDecorator = Cast<UBTDecorator_UtilityBlackboard>(Decorator))
			{
				BehaviorTreeComponent->RequestExecution(BlackboardUtilityDecorator);
				return;
			}
		}
	}
}

uint16 UBTComposite_Utility::GetInstanceMemorySize() const
{
	return sizeof(FBTUtilityMemory);
}

FString UBTComposite_Utility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Select child node with the highest score\nReevaluate on success: %s"),
		bReevaluateOnBranchSucceeded ? TEXT("true") : TEXT("false"));
}

void FIndexedUtilityValue::SetInvalid()
{
	ChildIdx = -1;
	UtilityScore = -FLT_MAX;
}
