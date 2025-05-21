#include "BehaviorTree/Tasks/BTTask_Gesture.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "NativeGameplayTags.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_Gesture::UBTTask_Gesture()
{
	NodeName = "Gesture";
	ActivityGestureBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_Gesture, ActivityGestureBBKey)));
	ActivityGestureBBKey.AllowNoneAsValue(true);
}

EBTNodeResult::Type UBTTask_Gesture::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
	FGameplayTag ActualGestureTag = GestureActionTag;
	if (!ActivityGestureBBKey.IsNone())
	{
		auto Blackboard = OwnerComp.GetBlackboardComponent();
		FGameplayTagContainer ActivityGestureTag = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(ActivityGestureBBKey.GetSelectedKeyID());
		ActualGestureTag = ActivityGestureTag.First();
	}
	
	if (Npc && Npc->PerformNpcGesture(ActualGestureTag))
	{
		if (bAwaitCompletion)
		{
			Super::ExecuteTask(OwnerComp, NodeMemory);
			return EBTNodeResult::InProgress;
		}

		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}

void UBTTask_Gesture::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Gesture_Completed;
	if (auto BBAsset = Asset.GetBlackboardAsset())
	{
		ActivityGestureBBKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_Gesture::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
		Npc->StopNpcGesture();
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

FString UBTTask_Gesture::GetStaticDescription() const
{
	return bAwaitCompletion
		? ActivityGestureBBKey.IsNone()
				? FString::Printf(TEXT("Perform gesture %s and wait\n%s" ), *GestureActionTag.ToString(), *Super::GetStaticDescription())
				: FString::Printf(TEXT("Perform activity gesture %s and wait\n%s" ), *ActivityGestureBBKey.SelectedKeyName.ToString(), *Super::GetStaticDescription())
		: ActivityGestureBBKey.IsNone()
			? FString::Printf(TEXT("Perform gesture %s\n%s" ), *GestureActionTag.ToString(), *Super::GetStaticDescription())
			: FString::Printf(TEXT("Perform activity gesture %s\n%s" ), *ActivityGestureBBKey.SelectedKeyName.ToString(), *Super::GetStaticDescription());

}
