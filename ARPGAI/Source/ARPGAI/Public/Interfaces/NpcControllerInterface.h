// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/NpcComponent.h"
#include "UObject/Interface.h"
#include "NpcControllerInterface.generated.h"

struct FAttackingThreatData;
struct FNpcThreatData;
struct FNpcActiveTargetData;

// This class does not need to be modified.
UINTERFACE()
class UNpcControllerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcControllerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void InjectDynamicBehaviors(UBTCompositeNode* StartingNode) = 0;
	virtual const TMap<FGameplayTag, UBehaviorTree*>* GetDynamicBehaviors() const = 0;
	virtual bool IsWantToAvoidThreats() const = 0;
	virtual float GetPathfindingDesiredAvoidThreatsDistance() const = 0;
	virtual float GetPathfindingAvoidThreatsScoreFactor() const = 0;
	virtual const FNpcActiveThreatsContainer& GetThreats() const = 0;
	virtual bool IsSurroundingTarget() const = 0;
	virtual void SetEnabled(bool bEnabled) = 0;
	virtual uint8 GetMaxFocusPriority() const = 0;
	virtual const TSubclassOf<UNavigationQueryFilter>& GetNpcDefaultNavigationFilterClass() const = 0;
	virtual void SetNavigationFilterClass(const TSubclassOf<UNavigationQueryFilter>& NavigationQueryFilterClass) = 0;
	virtual const FGameplayTag& GetDayTime() const = 0;
};