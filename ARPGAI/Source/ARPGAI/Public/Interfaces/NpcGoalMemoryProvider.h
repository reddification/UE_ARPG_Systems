// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcGoalMemoryProvider.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcGoalMemoryProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcGoalMemoryProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual uint8* AllocateGoalMemory(const FGuid& GoalId, size_t Size) = 0;
	virtual uint8* GetGoalMemory(const FGuid& GoalId) = 0;
	virtual void ClearGoalMemory(const FGuid& GoalId) = 0; 
};
