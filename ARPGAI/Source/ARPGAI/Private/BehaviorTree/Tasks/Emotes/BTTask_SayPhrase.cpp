#include "BehaviorTree/Tasks/Emotes/BTTask_SayPhrase.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Interfaces/NpcEmoteInterface.h"
#include "Perception/AISense_Hearing.h"

UBTTask_SayPhrase::UBTTask_SayPhrase()
{
	NodeName = "Say phrase";
	PhraseTagBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_SayPhrase, PhraseTagBBKey)));
	PhraseTagBBKey.AllowNoneAsValue(true);
}

EBTNodeResult::Type UBTTask_SayPhrase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	FGameplayTag ActualSpeechTag = DetermineActualSpeechTag(OwnerComp);
	ensure(ActualSpeechTag.IsValid());
	
	if (bReportNoiseEvent)
		UAISense_Hearing::ReportNoiseEvent(Pawn, Pawn->GetActorLocation(), Loudness, Pawn, Range, ActualSpeechTag.GetTagName());
	
	if (auto Npc = Cast<INpcEmoteInterface>(Pawn))
		Npc->SayPhrase_NPC(ActualSpeechTag);
	
	return EBTNodeResult::Succeeded;
}

FGameplayTag UBTTask_SayPhrase::DetermineActualSpeechTag(UBehaviorTreeComponent& OwnerComp)
{
	FGameplayTag ActualSpeechTag;
	if (!PhraseTagBBKey.IsNone())
	{
		auto Blackboard = OwnerComp.GetBlackboardComponent();
		ActualSpeechTag = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(PhraseTagBBKey.SelectedKeyName).First();
		ensure(ActualSpeechTag.IsValid());
	}
	
	if (!ActualSpeechTag.IsValid())
	{
		if (!PhraseOptions.IsEmpty())
		{
			if (PhraseOptions.Num() == 1)
				ActualSpeechTag = PhraseOptions.First();
			else 
				ActualSpeechTag = PhraseOptions.GetGameplayTagArray()[FMath::RandRange(0, PhraseOptions.Num() - 1)];
		}
	}
	
	return ActualSpeechTag;
}


FString UBTTask_SayPhrase::GetStaticDescription() const
{
	FString Description;
	if (!PhraseTagBBKey.IsNone())
	{
		Description = *FString::Printf(TEXT("From BB: %s"), *PhraseTagBBKey.SelectedKeyName.ToString());
	}
	else
	{
		FString PhraseList;
		for (const auto& PhraseTag : PhraseOptions)
			PhraseList += FString::Printf(TEXT("\n%s"), *PhraseTag.ToString());
		
		Description = TEXT("One of") + PhraseList;
	}
	
	if (bReportNoiseEvent)
		Description += FString::Printf(TEXT("\nReport noise event\nLoudness = %.2f\nRange = %.2f"), Loudness, Range);
	
	return Description;
}

void UBTTask_SayPhrase::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		PhraseTagBBKey.ResolveSelectedKey(*BB);
	}
}
