#include "Components/QuestGiverComponent.h"

#include "Subsystems/QuestSubsystem.h"

void UQuestGiverComponent::GiveQuests()
{
	auto QuestSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuestSubsystem>();
	for (const auto& QuestDTR: QuestsDTRHs)
		QuestSubsystem->StartQuest(QuestDTR);
}

void UQuestGiverComponent::StartQuests(TArray<FDataTableRowHandle>& Quests)
{
	// TODO use TSet instead of TArray
	for (const auto& QuestDTRH : Quests)
		QuestsDTRHs.AddUnique(QuestDTRH);
}
