#include "BehaviorTree/Tasks/Emotes/BTTask_Gesture.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "NativeGameplayTags.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/NpcEmoteInterface.h"

UBTTask_Gesture::UBTTask_Gesture()
{
	NodeName = "Gesture";
	ActivityGestureBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_Gesture, ActivityGestureBBKey)));
	ActivityGestureBBKey.AllowNoneAsValue(true);
}

EBTNodeResult::Type UBTTask_Gesture::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Npc = Cast<INpcEmoteInterface>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Npc)
		return EBTNodeResult::Failed;
	
	FGameplayTag ActualGestureTag = DetermineActualGestureTag(OwnerComp);
	if (!ActualGestureTag.IsValid())
		return EBTNodeResult::Failed;
	
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	if (Npc->PerformGesture_NPC(ActualGestureTag))
		return bAwaitCompletion ? EBTNodeResult::InProgress : EBTNodeResult::Succeeded;
	
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
	if (auto Npc = Cast<INpcEmoteInterface>(OwnerComp.GetAIOwner()->GetPawn()))
		Npc->StopGesture_NPC();
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

FGameplayTag UBTTask_Gesture::DetermineActualGestureTag(UBehaviorTreeComponent& OwnerComp) const
{
	FGameplayTag ActualGestureTag;
	if (!ActivityGestureBBKey.IsNone())
	{
		auto Blackboard = OwnerComp.GetBlackboardComponent();
		FGameplayTagContainer ActivityGestureTags = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(ActivityGestureBBKey.GetSelectedKeyID());
		const auto& GestureOptionsArray = ActivityGestureTags.GetGameplayTagArray();
		if (GestureOptionsArray.Num() > 0)
		{
			ActualGestureTag = GestureOptionsArray.Num() == 1
								   ? GestureOptionsArray[0]
								   : GestureOptionsArray[FMath::RandRange(0, GestureOptionsArray.Num() - 1)];
		}
	}
	else
	{
		int GestureOptionsCount = GestureOptions.Num();
		if (GestureOptionsCount > 0)
		{
			ActualGestureTag = GestureOptionsCount == 1 
				? GestureOptions.First()
				: GestureOptions.GetGameplayTagArray()[FMath::RandRange(0, GestureOptionsCount - 1)];
		}
		else
		{
			ActualGestureTag = GestureActionTag;
		}
	}
	
	return ActualGestureTag;
}

FString UBTTask_Gesture::GetStaticDescription() const
{
	FString AwaitClause = bAwaitCompletion ? TEXT("\nAwait for completion") : TEXT("");
	FString Result = TEXT("No gesture selected");
	if (ActivityGestureBBKey.IsNone())
	{
		const int GestureOptionsCount = GestureOptions.Num();
		if (GestureOptionsCount > 0)
		{
			FString GesturesList;
			for (const auto& GestureOption : GestureOptions)
				GesturesList += FString::Printf(TEXT("\n%s"), *GestureOption.ToString());
			
			Result = FString::Printf(TEXT("Perform random gesture from%s%s"), *GesturesList, *AwaitClause);
		}
		else if (GestureActionTag.IsValid())
		{
			Result = FString::Printf(TEXT("Perform gesture %s%s"), *GestureActionTag.ToString(), *AwaitClause);
		}
	}
	else
	{
		Result = FString::Printf(TEXT("Perform random gesture from BB\n%s%s"), *ActivityGestureBBKey.SelectedKeyName.ToString(), *AwaitClause);
	}
	
	return Result;
}
