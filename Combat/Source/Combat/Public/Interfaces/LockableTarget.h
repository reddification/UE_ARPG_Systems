// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LockableTarget.generated.h"

// This class does not need to be modified.
UINTERFACE()
class COMBAT_API UTargetable : public UInterface
{
	GENERATED_BODY()
};

class COMBAT_API ITargetable
{
	GENERATED_BODY()

public:
	virtual void SetTargetLockedOn(bool bLocked) = 0;
	virtual bool CanTarget(AActor* Caller) const = 0;
};
