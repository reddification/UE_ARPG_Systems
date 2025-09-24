// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QuestSystemContextProvider.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UQuestSystemContextProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class QUESTSYSTEM_API IQuestSystemContextProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual struct FQuestSystemContext GetQuestSystemContext() const = 0;
};
