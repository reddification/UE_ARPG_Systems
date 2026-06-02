#include "BehaviorTree/Tasks/Emotes/BTTask_SayPhrase_Selector.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SayPhrase_Selector::UBTTask_SayPhrase_Selector()
{
	NodeName = "Say phrase (selector)";
	ScalarSelectorBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SayPhrase_Selector, ScalarSelectorBBKey));
}

FGameplayTag UBTTask_SayPhrase_Selector::DetermineActualSpeechTag(UBehaviorTreeComponent& OwnerComp)
{
	float ScalarValue = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(ScalarSelectorBBKey.SelectedKeyName);
	for (const auto& OptionKVP : Options)
		if (OptionKVP.Value <= ScalarValue)
			return OptionKVP.Tag;
	
	return FallbackOption;
}

FString UBTTask_SayPhrase_Selector::GetStaticDescription() const
{
	if (Options.IsEmpty())
		TEXT("Set options!");
	
	FString Result = TEXT("Say");
	FString BBKeyName = ScalarSelectorBBKey.SelectedKeyName.ToString();
	for (const auto& OptionKVP : Options)
		Result += FString::Printf(TEXT("\n%s if %s <= %.2f"), *OptionKVP.Tag.ToString(), *BBKeyName, OptionKVP.Value);
	
	return Result;
}
