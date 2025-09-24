// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DelayedQuestActionInterface.generated.h"

struct FQuestSystemContext;

// This class does not need to be modified.
UINTERFACE()
class UDelayedQuestAction : public UInterface
{
	GENERATED_BODY()
};

class QUESTSYSTEM_API IDelayedQuestAction
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void StartDelayedAction(const FQuestSystemContext& QuestSystemContext) = 0;
};
