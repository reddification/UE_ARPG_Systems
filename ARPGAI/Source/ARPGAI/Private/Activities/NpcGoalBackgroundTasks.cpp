// 


#include "Activities/NpcGoalBackgroundTasks.h"

#include "Components/NpcComponent.h"
#include "Components/Controller/NpcActivityComponent.h"
#include "Interfaces/Npc.h"
#include "Subsystems/NpcRegistrationSubsystem.h"

void UNpcGoalBackgroundTaskRealtimeDialogue::Start(UNpcActivityComponent* NpcActivityComponent, bool bFirstStart)
{
	Super::Start(NpcActivityComponent, bFirstStart);
	StartConversation(NpcActivityComponent, !bFirstStart);
}

void UNpcGoalBackgroundTaskRealtimeDialogue::Stop(UNpcActivityComponent* NpcActivityComponent)
{
	Super::Stop(NpcActivityComponent);
	if (auto NpcOwner = Cast<INpc>(NpcActivityComponent->GetNpcPawn()))
		NpcOwner->StopConversation();
}

void UNpcGoalBackgroundTaskRealtimeDialogue::StartConversation(UNpcActivityComponent* NpcActivityComponent, bool bResume) const
{
	auto NpcOwner = Cast<INpc>(NpcActivityComponent->GetNpcPawn());
	auto NpcSubsystem = UNpcRegistrationSubsystem::Get(NpcActivityComponent);
	const float ConversationStartSearchRange = 500.f;
	TArray<FGameplayTagQuery> SearchFilters;
	auto ConversationPartnersNpcComponents = NpcSubsystem->GetNpcsInRange(NpcActivityComponent->GetPawnLocation(), ConversationStartSearchRange, SearchFilters);
	TArray<AActor*> ConversationPartners;
	for (const auto* ConversationPartnerNpcComponent : ConversationPartnersNpcComponents)
		ConversationPartners.Add(ConversationPartnerNpcComponent->GetOwner());
		
	NpcOwner->StartConversation(ConversationId, ConversationPartners, bResume);
}
