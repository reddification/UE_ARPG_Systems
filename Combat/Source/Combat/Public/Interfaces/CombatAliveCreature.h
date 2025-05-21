// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatAliveCreature.generated.h"

// This class does not need to be modified.
UINTERFACE()
class COMBAT_API UCombatAliveCreature : public UInterface
{
	GENERATED_BODY()
};

class COMBAT_API ICombatAliveCreature
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatCreatureDeadEvent, AActor* Creature);
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	FOnCombatCreatureDeadEvent OnCombatCreatureDeadEvent;
	
	virtual float GetCombatantHealth() const = 0;
};
