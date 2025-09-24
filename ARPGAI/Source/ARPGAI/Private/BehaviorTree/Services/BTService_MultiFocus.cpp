// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_MultiFocus.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTService_MultiFocus::UBTService_MultiFocus()
{
	NodeName = "Set multi-focus";

	bTickIntervals = false;
	bNotifyTick = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_MultiFocus::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	AAIController* OwnerController = OwnerComp.GetAIOwner();
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();

	if (OwnerController == nullptr || MyBlackboard == nullptr)
		return;

	for (int i = PrioritizedFocusPointsBBKeys.Num() - 1; i >= 0; i--)
	{
		const auto& BlackboardKey = PrioritizedFocusPointsBBKeys[i];
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID());
			if (AActor* TargetActor = Cast<AActor>(KeyValue))
				OwnerController->SetFocus(TargetActor, BasePriority + i);
		}
		else
		{
			const FVector FocusLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
			OwnerController->SetFocalPoint(FocusLocation, BasePriority + i);
		}

		const FBlackboard::FKey KeyID = BlackboardKey.GetSelectedKeyID();
		MyBlackboard->RegisterObserver(KeyID, this, FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_MultiFocus::OnBlackboardKeyValueChange));
	}
}

void UBTService_MultiFocus::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	
	AAIController* OwnerController = OwnerComp.GetAIOwner();
	if (OwnerController != nullptr)
	{
		for (int i = PrioritizedFocusPointsBBKeys.Num() - 1; i >= 0; i--)
			OwnerController->ClearFocus(BasePriority + i);
	}

	if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
		BlackboardComp->UnregisterObserversFrom(this);
}

EBlackboardNotificationResult UBTService_MultiFocus::OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard,
	FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	AAIController* OwnerController = OwnerComp ? OwnerComp->GetAIOwner() : nullptr;
	if (OwnerController == nullptr)
		return EBlackboardNotificationResult::RemoveObserver;

	const int32 NodeInstanceIdx = OwnerComp->FindInstanceContainingNode(this);
	for (int i = 0; i < PrioritizedFocusPointsBBKeys.Num(); i++)
	{
		const auto& BlackboardKey = PrioritizedFocusPointsBBKeys[i];
		if (BlackboardKey.GetSelectedKeyID() == ChangedKeyID)
		{
			if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
			{
				UObject* KeyValue = Blackboard.GetValue<UBlackboardKeyType_Object>(ChangedKeyID);
				AActor* TargetActor = Cast<AActor>(KeyValue);
				OwnerController->SetFocus(TargetActor, BasePriority + i);
			}
			else
			{
				const FVector FocusLocation = Blackboard.GetValue<UBlackboardKeyType_Vector>(ChangedKeyID);
				OwnerController->SetFocalPoint(FocusLocation, BasePriority + i);
			}

			break;
		}		
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTService_MultiFocus::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
		for (auto& BBKey : PrioritizedFocusPointsBBKeys)
			BBKey.ResolveSelectedKey(*BB);
}

FString UBTService_MultiFocus::GetStaticDescription() const
{
	FString Result = PrioritizedFocusPointsBBKeys.IsEmpty() ? TEXT("No focus points provided") : TEXT("Focus on points (low to high priority)");
	for (const auto& BBKey : PrioritizedFocusPointsBBKeys)
		Result = Result.Append(FString::Printf(TEXT("\n%s"), BBKey.SelectedKeyName.IsNone() ? TEXT("ERROR No BB specified") : *BBKey.SelectedKeyName.ToString()));
	
	return Result;
}

#if WITH_EDITOR

void UBTService_MultiFocus::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.MemberProperty &&
		PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UBTService_MultiFocus, PrioritizedFocusPointsBBKeys))
	{
		int Index = 0;
		for (auto& BBKey : PrioritizedFocusPointsBBKeys)
		{
			if (BBKey.AllowedTypes.IsEmpty())
			{
				BBKey.AddObjectFilter(this, FName(FString::Printf(TEXT("BTService_MultiFocus_BBKey_Actor_%d"), Index)), AActor::StaticClass());
				BBKey.AddVectorFilter(this, FName(FString::Printf(TEXT("BTService_MultiFocus_BBKey_Vector_%d"), Index)));
				Index++;
			}
		}
	}
}

FName UBTService_MultiFocus::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Service.DefaultFocus.Icon");
}

#endif	// WITH_EDITOR

