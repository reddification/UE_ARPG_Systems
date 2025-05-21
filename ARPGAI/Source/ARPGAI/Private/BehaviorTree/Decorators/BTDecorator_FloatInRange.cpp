// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_FloatInRange.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_FloatInRange::UBTDecorator_FloatInRange()
{
	NodeName = "Float in range";
	FlowAbortMode = EBTFlowAbortMode::Both;
	bAllowAbortChildNodes = true;
	bAllowAbortLowerPri = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	FloatBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_FloatInRange, FloatBBKey));
}

bool UBTDecorator_FloatInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const float GateParameterValue = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(FloatBBKey.SelectedKeyName);
	return GateParameterValue >= Min && GateParameterValue <= Max;
}

void UBTDecorator_FloatInRange::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		FloatBBKey.ResolveSelectedKey(*BB);
	}
}

void UBTDecorator_FloatInRange::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this,
						&UBTDecorator_FloatInRange::OnBlackboardKeyValueChange);
	OwnerComp.GetBlackboardComponent()->RegisterObserver(FloatBBKey.GetSelectedKeyID(), this, ObserverDelegate);
}

void UBTDecorator_FloatInRange::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->UnregisterObserversFrom(this);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTDecorator_FloatInRange::OnBlackboardKeyValueChange(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (BehaviorComp == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	if (FloatBBKey.GetSelectedKeyID() == Key)
	{	
		float NewGateParameterValue = BlackboardComponent.GetValueAsFloat(FloatBBKey.SelectedKeyName);

		if (NewGateParameterValue <= Max && NewGateParameterValue >= Min)
		{
			BehaviorComp->RequestExecution(this);
			return EBlackboardNotificationResult::RemoveObserver;
		}
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}

FString UBTDecorator_FloatInRange::GetStaticDescription() const
{
	return FString::Printf(TEXT("%.2f <= %s <= %.2f"), Min, *FloatBBKey.SelectedKeyName.ToString(), Max);
}
