#include "Data/NpcGoalItem.h"

#include "Activities/NpcGoals.h"

FString FNpcGoalItem::GetDescription() const
{
	if (NpcGoal)
		return FString::Printf(TEXT("%s\n"), *NpcGoal->GetDescription());
	else
		return TEXT("Goal not set");
}
