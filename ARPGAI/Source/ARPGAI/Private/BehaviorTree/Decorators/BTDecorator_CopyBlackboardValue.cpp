// 


#include "BehaviorTree/Decorators/BTDecorator_CopyBlackboardValue.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "Data/LogChannels.h"

UBTDecorator_CopyBlackboardValue::UBTDecorator_CopyBlackboardValue()
{
	NodeName = "Copy blackboard value";
	bNotifyActivation = true;
}

void UBTDecorator_CopyBlackboardValue::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (SourceBBKey.SelectedKeyType != DestinationBBKey.SelectedKeyType)
	{
		UE_VLOG(SearchData.OwnerComp.GetAIOwner(), LogARPGAI, Warning, TEXT("UBTDecorator_CopyBlackboardValue: source and destination BB key types don't match"));
		return;
	}

	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	auto OnSourceKeyChangedObserverCallback = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_CopyBlackboardValue::OnSourceChanged);
	Blackboard->RegisterObserver(SourceBBKey.GetSelectedKeyID(), this, OnSourceKeyChangedObserverCallback);
	Blackboard->CopyKeyValue(SourceBBKey.GetSelectedKeyID(), DestinationBBKey.GetSelectedKeyID());
}

EBlackboardNotificationResult UBTDecorator_CopyBlackboardValue::OnSourceChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	if (Key != SourceBBKey.GetSelectedKeyID())
		return EBlackboardNotificationResult::RemoveObserver;

	UBlackboardComponent& MutableBlackboard = const_cast<UBlackboardComponent&>(BlackboardComponent);
	MutableBlackboard.CopyKeyValue(SourceBBKey.GetSelectedKeyID(), DestinationBBKey.GetSelectedKeyID());
	
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_CopyBlackboardValue::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BBAsset = Asset.GetBlackboardAsset())
	{
		SourceBBKey.ResolveSelectedKey(*BBAsset);
		DestinationBBKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTDecorator_CopyBlackboardValue::GetStaticDescription() const
{
	return FString::Printf(TEXT("Copy %s to %s"), *SourceBBKey.SelectedKeyName.ToString(), *DestinationBBKey.SelectedKeyName.ToString());
}
