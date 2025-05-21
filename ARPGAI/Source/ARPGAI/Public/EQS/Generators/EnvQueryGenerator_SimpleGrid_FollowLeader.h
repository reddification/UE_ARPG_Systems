// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_SimpleGrid.h"
#include "EnvQueryGenerator_SimpleGrid_FollowLeader.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryGenerator_SimpleGrid_FollowLeader : public UEnvQueryGenerator_SimpleGrid
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_SimpleGrid_FollowLeader();
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UEnvQueryContext> FollowTargetContext;

	virtual FText GetDescriptionTitle() const override;
};
