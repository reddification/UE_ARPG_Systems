// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_Cone.h"
#include "EnvQueryGenerator_Cone_Extended.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryGenerator_Cone_Extended : public UEnvQueryGenerator_Cone
{
	GENERATED_BODY()

public:
	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

protected:
	UPROPERTY(EditDefaultsOnly)
	FAIDataProviderFloatValue ContextRotationOffsetValue;
};
