// 


#include "FlowGraph/Addons/Predicates/FlowNodeAddon_IsDayTime.h"

#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"

bool UFlowNodeAddon_IsDayTime::EvaluatePredicate_Implementation() const
{
	auto GameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	if (ensure(GameMode))
		return DayTimes.HasTagExact(GameMode->GetDayTime());

	return false;
}

#if WITH_EDITOR

FText UFlowNodeAddon_IsDayTime::GetNodeConfigText() const
{
	return FText::FromString(DayTimes.ToStringSimple());
}

#endif