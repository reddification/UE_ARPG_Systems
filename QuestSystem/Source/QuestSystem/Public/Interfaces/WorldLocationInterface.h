// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/QuestGiverComponent.h"
#include "UObject/Interface.h"
#include "WorldLocationInterface.generated.h"

class UBoxComponent;
// This class does not need to be modified.
UINTERFACE()
class QUESTSYSTEM_API UWorldLocationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class QUESTSYSTEM_API IWorldLocationInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	UBoxComponent* GetOverlapCollision() const;

	virtual UQuestGiverComponent* GetQuestGiverComponent() const { return nullptr; };
};
