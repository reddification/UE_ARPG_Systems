#include "BehaviorTree/Decorators/BTDecorator_GrantTagsOnCompletion.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcComponent.h"

UBTDecorator_GrantTagsOnCompletion::UBTDecorator_GrantTagsOnCompletion()
{
	NodeName = "Grant tags on completion";
	bNotifyDeactivation = true;
}

void UBTDecorator_GrantTagsOnCompletion::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	Super::OnNodeDeactivation(SearchData, NodeResult);
	auto NpcComponent = GetNpcComponent(SearchData.OwnerComp);
	if (NpcComponent == nullptr)
		return;
	
	if (!AlwaysGranted.Tags.IsEmpty())
	{
		if (AlwaysGranted.ForDurationGTH.IsSet())
			NpcComponent->GrantTempTags(AlwaysGranted.Tags, AlwaysGranted.ForDurationGTH.GetValue());
		else 
			NpcComponent->GrantTags(AlwaysGranted.Tags);
	}
		
	if (auto GrantedTagsOption = ConditionallyGrantedTags.Find(NodeResult))
	{
		if (!GrantedTagsOption->Tags.IsEmpty())
		{
			if (GrantedTagsOption->ForDurationGTH.IsSet())
				NpcComponent->GrantTempTags(GrantedTagsOption->Tags, GrantedTagsOption->ForDurationGTH.GetValue());
			else 
				NpcComponent->GrantTags(GrantedTagsOption->Tags);
		}
	}
}

FString UBTDecorator_GrantTagsOnCompletion::GetStaticDescription() const
{
	if (AlwaysGranted.Tags.IsEmpty() && ConditionallyGrantedTags.IsEmpty())
		return TEXT("No tags specified!");
	
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
	
	FString Result = TEXT("On deactivation\n");
	
	if (!AlwaysGranted.Tags.IsEmpty())
	{
		FString TagsList = FString::Printf(TEXT("Always grant%s:"), 
			AlwaysGranted.ForDurationGTH.IsSet() ? *FString::Printf(TEXT(" for %.2f game time hours"), AlwaysGranted.ForDurationGTH.GetValue()) : TEXT(""));
		
		for (const auto& Tag : AlwaysGranted.Tags)
			TagsList += "\n" + Tag.ToString();
		
		Result += TagsList;
	}
	
	for (const auto& ConditionalOption : ConditionallyGrantedTags)
	{
		FString ConditionOptionString = FString::Printf(TEXT("\nIf %s, grant%s:"),
			GetNodeResultName(ConditionalOption.Key), 
			ConditionalOption.Value.ForDurationGTH.IsSet() ? *FString::Printf(TEXT(" for %.2f game time hours"), ConditionalOption.Value.ForDurationGTH.GetValue()) : TEXT(""));
		
		for (const auto& Tag : ConditionalOption.Value.Tags)
			ConditionOptionString += "\n" + Tag.ToString();
		
		Result += ConditionOptionString;
	}

	return Result;
}
