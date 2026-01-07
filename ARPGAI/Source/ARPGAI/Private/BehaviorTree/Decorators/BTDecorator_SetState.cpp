// 


#include "BehaviorTree/Decorators/BTDecorator_SetState.h"

#include "AIHelpers.h"
#include "Activities/ActivityInstancesHelper.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_SetState::UBTDecorator_SetState()
{
	NodeName = "Set state";
	bNotifyTick = true;
	bTickIntervals = true;
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	FlowAbortMode = EBTFlowAbortMode::Self; // must be self in order to ::TickNode to be called
}

bool UBTDecorator_SetState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return true;
}

void UBTDecorator_SetState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	ensure(ActivationDelay > 0.f);
	auto DecoratorMemory = reinterpret_cast<FBTMemory_SetState*>(NodeMemory);
	DecoratorMemory->bDelayFinished = true;
	SetNextTickTime(NodeMemory, FLT_MAX);
	if (!bUseCondition || EvaluateOnBlackboard(*OwnerComp.GetBlackboardComponent()))
	{
		if (auto* NpcComponent = GetNpcComponent(OwnerComp))
		{
			NpcComponent->SetStateActive(NewStateTag, SetByCallerParams, true);
			DecoratorMemory->bStateApplied = true;
		}
	}
}

EBlackboardNotificationResult UBTDecorator_SetState::OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard,
                                                                                FBlackboard::FKey ChangedKeyID)
{
	if (!bUseCondition)
		return EBlackboardNotificationResult::RemoveObserver;
	
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	uint8* RawMemory = BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(this));
	auto NodeMemory = reinterpret_cast<FBTMemory_SetState*>(RawMemory);
	if (NodeMemory->bDelayFinished)
	{
		auto* NpcComponent = GetNpcComponent(*BehaviorComp);
		if (!NpcComponent)
		{
			ensure(false);
			return EBlackboardNotificationResult::RemoveObserver;
		}
		
		bool bConditionPasses = EvaluateOnBlackboard(Blackboard);
		if (bConditionPasses && !NodeMemory->bStateApplied)
		{
			NpcComponent->SetStateActive(NewStateTag, SetByCallerParams, true);
			NodeMemory->bStateApplied = true;
		}
		else if (!bConditionPasses && NodeMemory->bStateApplied)
		{
			NpcComponent->SetStateActive(NewStateTag, SetByCallerParams, false);
			NodeMemory->bStateApplied = false;
		}
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_SetState::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (!NewStateTag.IsValid())
		return;

	auto NodeMemory = GetNodeMemory<FBTMemory_SetState>(SearchData);
	
	if (ActivationDelay > 0.f)
	{
		SetNextTickTime((uint8*)NodeMemory, ActivationDelay);
	}
	else if (UNpcComponent* NpcStateComponent = GetNpcComponent(SearchData.OwnerComp))
	{
		SetNextTickTime((uint8*)NodeMemory, FLT_MAX);
		NodeMemory->bDelayFinished = true;
		bool bCanApply = bUseCondition ? EvaluateOnBlackboard(*SearchData.OwnerComp.GetBlackboardComponent()) : true;
		if (bCanApply)
		{
			NpcStateComponent->SetStateActive(NewStateTag, SetByCallerParams, true);
			NodeMemory->bStateApplied = true;
		}
	}
	else
	{
		ensure(false);
	}
}

void UBTDecorator_SetState::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	if (NewStateTag.IsValid())
	{
		auto NodeMemory = GetNodeMemory<FBTMemory_SetState>(SearchData);
		if (NodeMemory->bStateApplied)
		{
			if (UNpcComponent* NpcStateComponent = GetNpcComponent(SearchData.OwnerComp))
			{
				NpcStateComponent->SetStateActive(NewStateTag, SetByCallerParams, false);
			}
		}
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_SetState::GetStaticDescription() const
{
	FString Result = NewStateTag.ToString();
	if (bUseCondition)
	{
		Result = Result.Append(FString::Printf(TEXT("\nApply only when %s"), *Super::GetStaticDescription()));
	}
	
	return ActivationDelay > 0.f
		? Result.Append(FString::Printf(TEXT("\nActivate with %.2fs delay"), ActivationDelay))
		: Result;
}
