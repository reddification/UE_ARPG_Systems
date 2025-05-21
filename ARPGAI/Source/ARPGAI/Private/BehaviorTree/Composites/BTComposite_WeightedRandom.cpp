


#include "BehaviorTree/Composites/BTComposite_WeightedRandom.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Decorators/BTDecorator_MaybeBlackboardBase.h"

UBTComposite_WeightedRandom::UBTComposite_WeightedRandom()
{
	NodeName = "Weighted Random";
}

int32 UBTComposite_WeightedRandom::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild,
	EBTNodeResult::Type LastResult) const
{
	if (PrevChild != BTSpecialChild::NotInitialized)
	{
		return BTSpecialChild::ReturnToParent;
	}
	
	if (UBlackboardComponent* BlackboardComponent = SearchData.OwnerComp.GetBlackboardComponent())
	{
		float TotalRandomWeight = 0.f;
		TArray<float> ChildrenWeights;
		ChildrenWeights.Reserve(Children.Num());
		for (const FBTCompositeChild& Child : Children)
		{
			for (const UBTDecorator* Decorator : Child.Decorators)
			{
				if (const UBTDecorator_MaybeBase* DecoratorMaybeBlackboardBase = Cast<UBTDecorator_MaybeBase>(Decorator))
				{
					float Weight = DecoratorMaybeBlackboardBase->GetProbability(BlackboardComponent);
					TotalRandomWeight += Weight;
					ChildrenWeights.Emplace(TotalRandomWeight);
					break;
				}
			}
		}

		const float Random = FMath::RandRange(0.f, TotalRandomWeight);
		for (int i = 0; i < Children.Num(); i++)
		{
			if (Random < ChildrenWeights[i])
			{
				return i;
			}
		}
	}

	return BTSpecialChild::ReturnToParent;
}

void UBTComposite_WeightedRandom::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	// Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	InitializeNodeMemory<FBTCompositeMemory>(NodeMemory, InitType);
}

void UBTComposite_WeightedRandom::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	// Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
	CleanupNodeMemory<FBTCompositeMemory>(NodeMemory, CleanupType);
}
