// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_SimpleGrid.h"
#include "EnvQueryGenerator_SimpleGridRelativeTo.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryGenerator_SimpleGridRelativeTo : public UEnvQueryGenerator_SimpleGrid
{
	GENERATED_BODY()

public:
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	TSubclassOf<UEnvQueryContext> StartRelativeTo;

	// if true - the grid will be generated in direction opposing vector from primary context to "relative to" context
	// useful for backing off queries
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	bool bInverseDirection = false;	
};
