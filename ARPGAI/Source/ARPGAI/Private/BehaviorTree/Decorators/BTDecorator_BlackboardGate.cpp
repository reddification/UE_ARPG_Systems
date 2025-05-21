// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_BlackboardGate.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_BlackboardGate::UBTDecorator_BlackboardGate()
{
	NodeName = "Blackboard gate";
	FlowAbortMode = EBTFlowAbortMode::Both;
	bAllowAbortChildNodes = true;
	bAllowAbortLowerPri = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	GateParameterBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_BlackboardGate, GateParameterBBKey));
}

bool UBTDecorator_BlackboardGate::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const float GateParameterValue = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(GateParameterBBKey.SelectedKeyName);
	return GateParameterValue > Close;// && GateParameterValue >= Open;
}

void UBTDecorator_BlackboardGate::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		GateParameterBBKey.ResolveSelectedKey(*BB);
	}
}

void UBTDecorator_BlackboardGate::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this,
						&UBTDecorator_BlackboardGate::OnBlackboardKeyValueChange);
	auto BTMemory = reinterpret_cast<FBTMemory_BlackboardGate*>(NodeMemory);
	BTMemory->OldGateValue = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(GateParameterBBKey.SelectedKeyName);
	OwnerComp.GetBlackboardComponent()->RegisterObserver(GateParameterBBKey.GetSelectedKeyID(), this, ObserverDelegate);
}

void UBTDecorator_BlackboardGate::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->UnregisterObserversFrom(this);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTDecorator_BlackboardGate::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	FBTMemory_BlackboardGate* BTMemory = GetNodeMemory<FBTMemory_BlackboardGate>(SearchData);
	BTMemory->bNodeActive = true;
}

void UBTDecorator_BlackboardGate::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	FBTMemory_BlackboardGate* BTMemory = GetNodeMemory<FBTMemory_BlackboardGate>(SearchData);
	BTMemory->bNodeActive = false;
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

EBlackboardNotificationResult UBTDecorator_BlackboardGate::OnBlackboardKeyValueChange(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (BehaviorComp == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	if (GateParameterBBKey.GetSelectedKeyID() == Key)
	{
		FBTMemory_BlackboardGate* BTMemory = reinterpret_cast<FBTMemory_BlackboardGate*>(BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(this)));
		const float OldValue = BTMemory->OldGateValue;
		const float NewGateParameterValue = BlackboardComponent.GetValueAsFloat(GateParameterBBKey.SelectedKeyName);
		BTMemory->OldGateValue = NewGateParameterValue;

		bool bRequestExecution = (BTMemory->bNodeActive && OldValue > NewGateParameterValue && NewGateParameterValue <= Close)
			|| (!BTMemory->bNodeActive && OldValue < NewGateParameterValue && NewGateParameterValue >= Open);

		if (bRequestExecution)
			BehaviorComp->RequestExecution(this);
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}

FString UBTDecorator_BlackboardGate::GetStaticDescription() const
{
	return FString::Printf(TEXT("%.2f <= %s <= %.2f"), Close, *GateParameterBBKey.SelectedKeyName.ToString(), Open);
}
