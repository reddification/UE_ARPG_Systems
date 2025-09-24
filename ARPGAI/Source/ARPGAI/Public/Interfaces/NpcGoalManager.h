// 

#pragma once

#include "CoreMinimal.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "UObject/Interface.h"
#include "NpcGoalManager.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcGoalManager : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcGoalManager
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ResumeGoal() = 0;
	virtual void SuspendActiveGoal() = 0;
	virtual ENpcGoalAdvanceResult AdvanceCurrentGoal(const FGameplayTagContainer& GoalExecutionResultTags) = 0;
	virtual bool SetActivityGoalData() = 0;
	virtual const FGameplayTag& GetGoalTagParameter(const FGameplayTag& ParameterId) const = 0;
};
