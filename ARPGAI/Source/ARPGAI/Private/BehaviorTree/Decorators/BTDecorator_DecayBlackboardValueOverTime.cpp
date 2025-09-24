// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_DecayBlackboardValueOverTime.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_DecayBlackboardValueOverTime::UBTDecorator_DecayBlackboardValueOverTime()
{
	NodeName = "Decay blackboard value over time";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	bTickIntervals = true;
	bNotifyTick = true;
	
	FlowAbortMode = EBTFlowAbortMode::Self;
	GateBBKey.AllowNoneAsValue(true);
	GateBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_DecayBlackboardValueOverTime, GateBBKey));
}

// Tick must be AFTER OnNodeActivation. If it doesnt work - try OnBecomeRelevant
void UBTDecorator_DecayBlackboardValueOverTime::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	OwnerComp.GetBlackboardComponent()->ClearValue(BlackboardKey.SelectedKeyName);
	SetNextTickTime(NodeMemory, FLT_MAX);
}

void UBTDecorator_DecayBlackboardValueOverTime::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto NodeMemory = GetNodeMemory<FBTMemory_DecayBlackboardValueOverTime>(SearchData);
	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	auto BlackboardKeyObserver = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_DecayBlackboardValueOverTime::OnBlackboardKeyValueChange);
	NodeMemory->BlackboardKeyChangedDelegateHandle = Blackboard->RegisterObserver(BlackboardKey.GetSelectedKeyID(), this, BlackboardKeyObserver);
	
	bool bCanDecay = true;
	if (GateBBKey.IsSet())
	{
		auto bGateOpened = Blackboard->GetValueAsBool(GateBBKey.SelectedKeyName);
		bCanDecay = bGateOpened ^ bGateInversed;
		auto GateObserver = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_DecayBlackboardValueOverTime::OnGateChanged);
		NodeMemory->GateChangedDelegateHandle = Blackboard->RegisterObserver(GateBBKey.GetSelectedKeyID(), this, GateObserver);
	}
	
	if (bCanDecay)
		SetNextTickTime(reinterpret_cast<uint8*>(NodeMemory), ResetDelayTime);
}

void UBTDecorator_DecayBlackboardValueOverTime::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto BTMemory = GetNodeMemory<FBTMemory_DecayBlackboardValueOverTime>(SearchData))
	{
		if (auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent())
		{
			if (BTMemory->BlackboardKeyChangedDelegateHandle.IsValid())
			{
				Blackboard->UnregisterObserver(BlackboardKey.GetSelectedKeyID(), BTMemory->BlackboardKeyChangedDelegateHandle);
				BTMemory->BlackboardKeyChangedDelegateHandle.Reset();
			}

			if (BTMemory->GateChangedDelegateHandle.IsValid())
			{
				Blackboard->UnregisterObserver(GateBBKey.GetSelectedKeyID(), BTMemory->GateChangedDelegateHandle);
				BTMemory->GateChangedDelegateHandle.Reset();
			}
		}
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

void UBTDecorator_DecayBlackboardValueOverTime::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		GateBBKey.ResolveSelectedKey(*BB);
		BlackboardKey.ResolveSelectedKey(*BB);
	}
}

EBlackboardNotificationResult UBTDecorator_DecayBlackboardValueOverTime::OnBlackboardKeyValueChange(
	const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	if (ChangedKeyID == BlackboardKey.GetSelectedKeyID())
	{
		UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
		if (BehaviorComp == nullptr)
			return EBlackboardNotificationResult::RemoveObserver;

		bool bGateOpened = true;
		if (GateBBKey.IsSet())
			bGateOpened = Blackboard.GetValueAsBool(GateBBKey.SelectedKeyName) ^ bGateInversed;
		
		if (bGateOpened)
		{
			int32 NodeIndex = BehaviorComp->FindInstanceContainingNode(this);
			uint8* RawMemory = BehaviorComp->GetNodeMemory(this, NodeIndex);
			SetNextTickTime(RawMemory, ResetDelayTime);
		}
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}

EBlackboardNotificationResult UBTDecorator_DecayBlackboardValueOverTime::OnGateChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	if (ensure(Key == GateBBKey.GetSelectedKeyID()))
	{
		UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
		if (BehaviorComp == nullptr)
			return EBlackboardNotificationResult::RemoveObserver;

		int32 NodeIndex = BehaviorComp->FindInstanceContainingNode(this);
		uint8* RawMemory = BehaviorComp->GetNodeMemory(this, NodeIndex);
	
		bool bGateOpened = BlackboardComponent.GetValueAsBool(GateBBKey.SelectedKeyName) ^ bGateInversed;
		if (bGateOpened)
			SetNextTickTime(RawMemory, ResetDelayTime);
		else
			SetNextTickTime(RawMemory, FLT_MAX);
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}

FString UBTDecorator_DecayBlackboardValueOverTime::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT("After %s is changed, wait %.2fs and then reset it"), *BlackboardKey.SelectedKeyName.ToString(), ResetDelayTime);
	if (!GateBBKey.SelectedKeyName.IsNone())
		Result = Result.Append(FString::Printf(TEXT("\nUse gate %s, decay only when it is %s"), *GateBBKey.SelectedKeyName.ToString(), bGateInversed ? TEXT("false") : TEXT("true")));

	return Result;
}
