// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_FloatFluctuation.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"

UBTService_FloatFluctuation::UBTService_FloatFluctuation()
{
	NodeName = "Float Fluctuation";
	FloatBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_FloatFluctuation, FloatBBKey));
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_FloatFluctuation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	FBTMemory_FluctuatingFloat* BTMemory = reinterpret_cast<FBTMemory_FluctuatingFloat*>(NodeMemory);
	if (BTMemory->InitialValue <= 0.f)
	{
		// this is required when service is set to tick on search start, in this case the 1st tick is executed before OnBecomeRelevant
		BTMemory->InitialValue = Blackboard->GetValueAsFloat(FloatBBKey.SelectedKeyName); 
	}
	
	const float NewValue = bUseFraction
		? FMath::RandRange(BTMemory->InitialValue - BTMemory->InitialValue * FluctuationNegative, BTMemory->InitialValue + BTMemory->InitialValue * FluctuationPositive)
		: FMath::RandRange(BTMemory->InitialValue - FluctuationNegative, BTMemory->InitialValue + FluctuationPositive);
	Blackboard->SetValue<UBlackboardKeyType_Float>(FloatBBKey.GetSelectedKeyID(), NewValue);
}

void UBTService_FloatFluctuation::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	FBTMemory_FluctuatingFloat* BTMemory = reinterpret_cast<FBTMemory_FluctuatingFloat*>(NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	if (BTMemory->InitialValue <= 0.f)
		BTMemory->InitialValue = Blackboard->GetValueAsFloat(FloatBBKey.SelectedKeyName);
	
	// makes no sense to observe it - the service itself changes this value
	// BTMemory->BlackboardObserverHandle = Blackboard->RegisterObserver(FloatBBKey.GetSelectedKeyID(), this,
	// 	FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_FloatFluctuation::OnBlackboardKeyChanged));
}

void UBTService_FloatFluctuation::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	FBTMemory_FluctuatingFloat* BTMemory = reinterpret_cast<FBTMemory_FluctuatingFloat*>(NodeMemory);
	Blackboard->SetValue<UBlackboardKeyType_Float>(FloatBBKey.GetSelectedKeyID(), BTMemory->InitialValue);
	// Blackboard->UnregisterObserver(FloatBBKey.GetSelectedKeyID(), BTMemory->BlackboardObserverHandle);
}

EBlackboardNotificationResult UBTService_FloatFluctuation::OnBlackboardKeyChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	if (!ensure(Key == FloatBBKey.GetSelectedKeyID()))
		return EBlackboardNotificationResult::ContinueObserving;

	const float NewValue = BlackboardComponent.GetValue<UBlackboardKeyType_Float>(FloatBBKey.GetSelectedKeyID());
	FBTMemory_FluctuatingFloat* BTMemory = nullptr;
	ensure(false); // TODO get uint8* NodeMemory here somehow
	return EBlackboardNotificationResult::ContinueObserving;
	// BTMemory->InitialValue = NewValue;
}

void UBTService_FloatFluctuation::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BBAsset = Asset.GetBlackboardAsset())
	{
		FloatBBKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTService_FloatFluctuation::GetStaticDescription() const
{
	return bUseFraction
		? FString::Printf(TEXT("Fluctuating %s -%d %% +%d %%\n%s"), *FloatBBKey.SelectedKeyName.ToString(),
			(int)(FluctuationNegative*100), (int)(FluctuationPositive*100), *Super::GetStaticDescription())
		: FString::Printf(TEXT("Fluctuating %s -%.2f +%.2f\n%s"), *FloatBBKey.SelectedKeyName.ToString(), FluctuationNegative, FluctuationPositive, *Super::GetStaticDescription());
}