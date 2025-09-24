// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_DecayBlackboardValueOverTime.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_DecayBlackboardValueOverTime::UBTService_DecayBlackboardValueOverTime()
{
	NodeName = "Decay blackboard value over time";

	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	
	GateBBKey.AllowNoneAsValue(true);
	GateBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_DecayBlackboardValueOverTime, GateBBKey));
}

void UBTService_DecayBlackboardValueOverTime::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	// Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds); // no need for super since it only calls SetNextTickTime
	OwnerComp.GetBlackboardComponent()->ClearValue(BlackboardKey.SelectedKeyName);
	SetNextTickTime(NodeMemory, FLT_MAX);
}

void UBTService_DecayBlackboardValueOverTime::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto BTMemory = reinterpret_cast<FBTMemory_DecayBlackboardValueOverTime*>(NodeMemory);
	auto BlackboardKeyObserver = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_DecayBlackboardValueOverTime::OnBlackboardKeyValueChange);
	BTMemory->BlackboardKeyChangedDelegateHandle = Blackboard->RegisterObserver(BlackboardKey.GetSelectedKeyID(), this, BlackboardKeyObserver);
	
	bool bCanDecay = true;
	if (GateBBKey.IsSet())
	{
		auto bGateOpened = Blackboard->GetValueAsBool(GateBBKey.SelectedKeyName);
		bCanDecay = bGateOpened ^ bGateInversed;
		auto GateObserver = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_DecayBlackboardValueOverTime::OnGateChanged);
		BTMemory->GateChangedDelegateHandle = Blackboard->RegisterObserver(GateBBKey.GetSelectedKeyID(), this, GateObserver);
	}
	
	if (bCanDecay)
		SetNextTickTime(NodeMemory, ResetDelayTime);
}

void UBTService_DecayBlackboardValueOverTime::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto BTMemory = reinterpret_cast<FBTMemory_DecayBlackboardValueOverTime*>(NodeMemory))
	{
		if (auto Blackboard = OwnerComp.GetBlackboardComponent())
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
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_DecayBlackboardValueOverTime::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		GateBBKey.ResolveSelectedKey(*BB);
		BlackboardKey.ResolveSelectedKey(*BB);
	}
}

EBlackboardNotificationResult UBTService_DecayBlackboardValueOverTime::OnBlackboardKeyValueChange(
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

EBlackboardNotificationResult UBTService_DecayBlackboardValueOverTime::OnGateChanged(
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

FString UBTService_DecayBlackboardValueOverTime::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT("After %s is changed, wait %.2fs and then reset it"), *BlackboardKey.SelectedKeyName.ToString(), ResetDelayTime);
	if (!GateBBKey.SelectedKeyName.IsNone())
		Result = Result.Append(FString::Printf(TEXT("\nUse gate %s, decay only when it is %s"), *GateBBKey.SelectedKeyName.ToString(), bGateInversed ? TEXT("false") : TEXT("true")));

	return Result;
}
