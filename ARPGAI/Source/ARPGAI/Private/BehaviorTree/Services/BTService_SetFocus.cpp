


#include "BehaviorTree/Services/BTService_SetFocus.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Interfaces/NpcControllerInterface.h"


UBTService_SetFocus::UBTService_SetFocus()
{
	NodeName = "Set focus";

	bNotifyTick = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	// accept only actors and vectors
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SetFocus, BlackboardKey), AActor::StaticClass());
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SetFocus, BlackboardKey));
}

void UBTService_SetFocus::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	INpcControllerInterface* NpcController = Cast<INpcControllerInterface>(OwnerComp.GetAIOwner());
	if(!NpcController)
	{
		return;
	}
	
	uint8 FocusPriority = NpcController->GetMaxFocusPriority();
	FBTMemory_AIFocus* MyMemory = CastInstanceNodeMemory<FBTMemory_AIFocus>(NodeMemory);
	MyMemory->Reset();
	MyMemory->ActualFocusPriority = FocusPriority;
	
	AAIController* OwnerController = OwnerComp.GetAIOwner();
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	
	if (OwnerController != nullptr && MyBlackboard != nullptr)
	{
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID());
			AActor* TargetActor = Cast<AActor>(KeyValue);
			if (TargetActor)
			{
				OwnerController->SetFocus(TargetActor, FocusPriority);
				MyMemory->FocusActorSet = TargetActor;
				MyMemory->bActorSet = true;
			}
		}
		else
		{
			const FVector FocusLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
			OwnerController->SetFocalPoint(FocusLocation, FocusPriority);
			MyMemory->FocusLocationSet = FocusLocation;
		}

		const FBlackboard::FKey KeyID = BlackboardKey.GetSelectedKeyID();
		MyBlackboard->RegisterObserver(KeyID, this, FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_SetFocus::OnBlackboardKeyValueChangeWithPriority));
	}
}

void UBTService_SetFocus::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTMemory_AIFocus* MyMemory = CastInstanceNodeMemory<FBTMemory_AIFocus>(NodeMemory);
	AAIController* OwnerController = OwnerComp.GetAIOwner();
	if (OwnerController != nullptr)
	{
		bool bClearFocus = false;
		if (MyMemory->bActorSet)
		{
			bClearFocus = (MyMemory->FocusActorSet == OwnerController->GetFocusActorForPriority(MyMemory->ActualFocusPriority));
		}
		else
		{
			bClearFocus = (MyMemory->FocusLocationSet == OwnerController->GetFocalPointForPriority(MyMemory->ActualFocusPriority));
		}

		if (bClearFocus)
		{
			OwnerController->ClearFocus(MyMemory->ActualFocusPriority);
		}
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->UnregisterObserversFrom(this);
	}
}

EBlackboardNotificationResult UBTService_SetFocus::OnBlackboardKeyValueChangeWithPriority(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	AAIController* OwnerController = OwnerComp ? OwnerComp->GetAIOwner() : nullptr;
	if (OwnerController == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	const int32 NodeInstanceIdx = OwnerComp->FindInstanceContainingNode(this);
	FBTMemory_AIFocus* MyMemory = CastInstanceNodeMemory<FBTMemory_AIFocus>(OwnerComp->GetNodeMemory(this, NodeInstanceIdx));
	OwnerController->ClearFocus(MyMemory->ActualFocusPriority);

	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		UObject* KeyValue = Blackboard.GetValue<UBlackboardKeyType_Object>(ChangedKeyID);
		AActor* TargetActor = Cast<AActor>(KeyValue);
		if (TargetActor)
		{
			OwnerController->SetFocus(TargetActor, MyMemory->ActualFocusPriority);
			MyMemory->FocusActorSet = TargetActor;
			MyMemory->bActorSet = true;
		}
	}
	else
	{
		const FVector FocusLocation = Blackboard.GetValue<UBlackboardKeyType_Vector>(ChangedKeyID);
		OwnerController->SetFocalPoint(FocusLocation, MyMemory->ActualFocusPriority);
		MyMemory->FocusLocationSet = FocusLocation;
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTService_SetFocus::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey.ResolveSelectedKey(*BBAsset);
	}
}

#if WITH_EDITOR

FName UBTService_SetFocus::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Service.DefaultFocus.Icon");
}

#endif	// WITH_EDITOR



FString UBTService_SetFocus::GetStaticDescription() const
{
	FString KeyDesc("invalid");
	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	}

	return FString::Printf(TEXT("Set default focus to %s"), *KeyDesc);
}
