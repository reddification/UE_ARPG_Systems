// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "NpcGameWorldTimeManager.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcGameWorldTimeManager : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcGameWorldTimeManager
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	DECLARE_MULTICAST_DELEGATE_OneParam(FNpcDayTimeChangedEvent, const FGameplayTag& NewDayTime);
	DECLARE_MULTICAST_DELEGATE_OneParam(FNpcGameWorldTimeSkippedEvent, const FDateTime& CurrentTime);
	
public:
	
	virtual const FDateTime& GetARPGAIGameTime() const = 0;
	virtual float GetTimeRateSeconds() const = 0;
	virtual float ConvertGameTimeToRealTime(float GameTimeDurationInHours) const = 0;
	virtual const FGameplayTag& GetDayTime() const = 0;

	FNpcDayTimeChangedEvent NpcDayTimeChangedEvent;
	FNpcGameWorldTimeSkippedEvent NpcGameWorldTimeSkippedEvent;
};
