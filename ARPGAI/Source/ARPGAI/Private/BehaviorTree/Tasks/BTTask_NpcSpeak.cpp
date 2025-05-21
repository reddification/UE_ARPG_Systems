// 

#include "BehaviorTree/Tasks/BTTask_NpcSpeak.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Perception/AISense_Hearing.h"

UBTTask_NpcSpeak::UBTTask_NpcSpeak()
{
	NodeName = "Say to NPCs";
	AISoundTagBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_NpcSpeak, AISoundTagBBKey)));
	AISoundTagBBKey.AllowNoneAsValue(true);
}

EBTNodeResult::Type UBTTask_NpcSpeak::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Controller = OwnerComp.GetAIOwner();
	auto Pawn = Controller->GetPawn();
	
	FGameplayTag ActualSpeechTag = AISoundTag;
	if (!AISoundTagBBKey.IsNone())
	{
		auto Blackboard = OwnerComp.GetBlackboardComponent();
		ActualSpeechTag = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(AISoundTagBBKey.SelectedKeyName).First();
		ensure(ActualSpeechTag.IsValid());
	}

	ensure(ActualSpeechTag.IsValid());
	UAISense_Hearing::ReportNoiseEvent(Pawn, Pawn->GetActorLocation(), Loudness, Pawn, Range, ActualSpeechTag.GetTagName());

	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	if (ensure(NpcGameMode))
	{
		auto NpcComponent = Pawn->FindComponentByClass<UNpcComponent>();
		NpcGameMode->ReportNpcSpeak(Pawn, NpcComponent->GetNpcIdTag(), ActualSpeechTag.IsValid () ? ActualSpeechTag : AISoundTag, Range * Loudness);
	}
	
	return EBTNodeResult::Succeeded;
}

FString UBTTask_NpcSpeak::GetStaticDescription() const
{
	return FString::Printf(TEXT("Report AI noise event %s\nLoudness = %.2f\nRange = %.2f"),
		AISoundTagBBKey.SelectedKeyName.IsNone() ? *AISoundTag.ToString() : *FString::Printf(TEXT("From BB: %s"), *AISoundTagBBKey.SelectedKeyName.ToString()),
		Loudness, Range);
}

void UBTTask_NpcSpeak::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		AISoundTagBBKey.ResolveSelectedKey(*BB);
	}
}
