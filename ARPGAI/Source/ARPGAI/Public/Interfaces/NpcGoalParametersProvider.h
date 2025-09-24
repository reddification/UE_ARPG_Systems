// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcGoalParametersProvider.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcGoalParametersProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcGoalParametersProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual struct FConstStructView GetParametersView() = 0;
};
