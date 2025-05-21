// 


#include "BehaviorTree/Tasks/BTTask_ChangeBlackboardGameplayTags.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ChangeBlackboardGameplayTags::UBTTask_ChangeBlackboardGameplayTags()
{
	NodeName = "Change blackboard gameplay tags";
	GameplayTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_ChangeBlackboardGameplayTags, GameplayTagsBBKey)));
}

EBTNodeResult::Type UBTTask_ChangeBlackboardGameplayTags::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	FGameplayTagContainer CurrentTags = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(GameplayTagsBBKey.SelectedKeyName);
	switch (ChangeBlackboardGameplayTagsType)
	{
		case EChangeBlackboardGameplayTagsType::Append:
			CurrentTags.AppendTags(DeltaTags);
			break;
		case EChangeBlackboardGameplayTagsType::Overwrite:
			CurrentTags = DeltaTags;
			break;
		case EChangeBlackboardGameplayTagsType::Remove:
			CurrentTags.RemoveTags(DeltaTags);
			break;
		default:
			ensure(false);
			break;
	}

	Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(GameplayTagsBBKey.SelectedKeyName, CurrentTags);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_ChangeBlackboardGameplayTags::GetStaticDescription() const
{
	switch (ChangeBlackboardGameplayTagsType)
	{
		case EChangeBlackboardGameplayTagsType::Append:
			return FString::Printf(TEXT("Append %s to %s"), *DeltaTags.ToStringSimple(), *GameplayTagsBBKey.SelectedKeyName.ToString());
		case EChangeBlackboardGameplayTagsType::Overwrite:
			return FString::Printf(TEXT("%s = %s"), *GameplayTagsBBKey.SelectedKeyName.ToString(), *DeltaTags.ToStringSimple());
		case EChangeBlackboardGameplayTagsType::Remove:
			return FString::Printf(TEXT("Remove %s from %s"), *DeltaTags.ToStringSimple(), *GameplayTagsBBKey.SelectedKeyName.ToString());
		default:
			return TEXT("WTF b0ss");
	}
}
