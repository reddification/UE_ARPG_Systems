#include "ARPGAI\Private\AIHelpers.h"

#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcGameWorldTimeManager.h"

FDateTime GetGameWorldTime(const UObject* WorldContextObject, float GameTimeHoursOffset)
{
	auto WorldTimeManager = Cast<INpcGameWorldTimeManager>(WorldContextObject->GetWorld()->GetAuthGameMode());
	return WorldTimeManager
		? WorldTimeManager->GetARPGAIGameTime() + FTimespan::FromHours(GameTimeHoursOffset) 
		: FDateTime(WorldContextObject->GetWorld()->GetTimeSeconds() + 60.f * 60.f * GameTimeHoursOffset); 
}
