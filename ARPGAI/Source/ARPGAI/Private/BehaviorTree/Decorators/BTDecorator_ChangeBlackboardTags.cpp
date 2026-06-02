#include "BehaviorTree/Decorators/BTDecorator_ChangeBlackboardTags.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_ChangeBlackboardTags::UBTDecorator_ChangeBlackboardTags()
{
	NodeName = "Change blackboard tags";
	TagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_ChangeBlackboardTags, TagsBBKey)));
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_ChangeBlackboardTags::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (!OnActivation.Tags.IsEmpty())
	{
		auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
		FGameplayTagContainer CurrentTags = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(TagsBBKey.SelectedKeyName);
		if (OnActivation.bAdd)
			CurrentTags.AppendTags(OnActivation.Tags);
		else 
			CurrentTags.RemoveTags(OnActivation.Tags);
		
		Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(TagsBBKey.SelectedKeyName, CurrentTags);
	}		
}

void UBTDecorator_ChangeBlackboardTags::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	Super::OnNodeDeactivation(SearchData, NodeResult);
	
	auto OnDeactivationHandler = [this](const FBlackboardKeyTagsOperation& OnDeactivation, UBlackboardComponent* Blackboard)
	{
		if (!OnDeactivation.Tags.IsEmpty())
		{
			if (IsValid(Blackboard))
			{
				FGameplayTagContainer CurrentTags = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(TagsBBKey.SelectedKeyName);
				if (OnDeactivation.bAdd)
					CurrentTags.AppendTags(OnDeactivation.Tags);
				else 
					CurrentTags.RemoveTags(OnDeactivation.Tags);
			
				Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(TagsBBKey.SelectedKeyName, CurrentTags);
			}
		}
	};

	if (auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent())
	{
		if (!OnDeactivationAlways.Tags.IsEmpty())
			OnDeactivationHandler(OnDeactivationAlways, Blackboard);
		
		if (OnDeactivationConditional.Contains(NodeResult))
			OnDeactivationHandler(OnDeactivationConditional[NodeResult], Blackboard);
	}
}

FString UBTDecorator_ChangeBlackboardTags::GetStaticDescription() const
{
	FString Description = FString::Printf(TEXT("Tags BB: %s"), *TagsBBKey.SelectedKeyName.ToString());
	if (!OnActivation.Tags.IsEmpty())
	{
		FString TagsList;
		for (const auto& Tag : OnActivation.Tags)
			TagsList += "\n" + Tag.ToString();
		
		Description += FString::Printf(TEXT("\nOn activation, %s%s"), OnActivation.bAdd ? TEXT("add") : TEXT("remove"), *TagsList);
	}
	
	auto GetNodeResultName = [](EBTNodeResult::Type NodeResult)
	{
		switch (NodeResult)
		{
		case EBTNodeResult::Aborted:
			return TEXT("aborted");
		case EBTNodeResult::Failed:
			return TEXT("failed");
		case EBTNodeResult::Succeeded:
			return TEXT("succeeded");
		default:
			return TEXT("unknown");
		};
	};
		
	if (!OnDeactivationAlways.Tags.IsEmpty() || !OnDeactivationConditional.IsEmpty())
	{
		FString OnDeactivationDescription = TEXT("On deactivation");
		if (!OnDeactivationAlways.Tags.IsEmpty())
		{
			OnDeactivationDescription += TEXT("\nAlways:");
			FString TagsList;
			for (const auto& Tag : OnDeactivationAlways.Tags)
				TagsList += "\n" + Tag.ToString();
		
			OnDeactivationDescription += FString::Printf(TEXT("%s%s"), OnDeactivationAlways.bAdd ? TEXT("add") : TEXT("remove"), *TagsList);
		}
		
		if (!OnDeactivationConditional.IsEmpty())
		{
			for (const auto& OnDeactivationKVP : OnDeactivationConditional)
			{
				const auto& OnDeactivation = OnDeactivationKVP.Value;
				if (!OnDeactivation.Tags.IsEmpty())
				{
					FString NodeResultName = GetNodeResultName(OnDeactivationKVP.Key);
				
					if (!Description.IsEmpty())
						Description += TEXT("\n\n");
			
					FString TagsList;
					for (const auto& Tag : OnDeactivation.Tags)
						TagsList += "\n" + Tag.ToString();
			
					Description += FString::Printf(TEXT("When %s, %s%s"), *NodeResultName, OnDeactivation.bAdd ? TEXT("add") : TEXT("remove"), *TagsList);
				}
			}
		}
		
		Description += TEXT("\n") + OnDeactivationDescription;
	}
	
	return Description;
}