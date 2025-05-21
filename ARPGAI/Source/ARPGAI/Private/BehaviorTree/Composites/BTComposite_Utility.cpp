
#include "BehaviorTree/Composites/BTComposite_Utility.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
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
	for(int32 Idx = 0; Idx < GetChildrenNum(); ++Idx)
	{
		if (Count > MAX_UTILITY_NODES)
		{
			UE_LOG(LogAIUtility, Warning, TEXT("[%s] BT utility composite contains more than %d utility children. Remaining won't get evaluated"), *GetTreeAsset()->GetName(), MAX_UTILITY_NODES)
			UtilityMemory->ExecutionUtilityOrdering[Idx].SetInvalid();
		}
		
		if (const UBTDecorator_UtilityFunction* UtilityDecorator = FindChildUtilityDecorator(Idx))
		{
			const float Score = UtilityDecorator
			   ? UtilityDecorator->WrappedCalculateUtility(SearchData.OwnerComp, UtilityDecorator->GetNodeMemory<uint8>(SearchData))
			   : 0.0f;

			UtilityMemory->ExecutionUtilityOrdering[Count].ChildIdx = Idx;
			UtilityMemory->ExecutionUtilityOrdering[Count].UtilityScore = Score;
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
	for (const FBTCompositeChild& Child : Children)
	{
		for (UBTDecorator* Decorator : Child.Decorators)
		{
			if (UBTDecorator_UtilityBlackboard* BlackboardUtilityDecorator = Cast<UBTDecorator_UtilityBlackboard>(Decorator))
			{
				if (UBlackboardComponent* BlackboardComp = SearchData.OwnerComp.GetBlackboardComponent())
				{
					FBlackboardKeySelector SelectedBlackboardKeySelector = BlackboardUtilityDecorator->GetSelectedBlackboardKeySelector();
					FBlackboard::FKey KeyID = SelectedBlackboardKeySelector.GetSelectedKeyID();
					FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(BlackboardUtilityDecorator,
						&UBTDecorator_UtilityBlackboard::OnBlackboardKeyValueChange);
					BlackboardComp->RegisterObserver(KeyID, BlackboardUtilityDecorator, ObserverDelegate);
				}

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
