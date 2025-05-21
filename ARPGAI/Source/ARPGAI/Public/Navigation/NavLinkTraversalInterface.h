// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RecastQueryARPGFilter.h"
#include "UObject/Interface.h"
#include "NavLinkTraversalInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNavLinkTraversalInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INavLinkTraversalInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetCostMultiplier(const AAIController* AIController) const = 0;
};
