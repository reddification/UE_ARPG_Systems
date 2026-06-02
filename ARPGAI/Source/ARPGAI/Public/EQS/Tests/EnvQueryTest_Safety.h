// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_Safety.generated.h"

/**
 * This test is bullshit. instead, search for a good safe place should be based on navmesh edges
 */
UCLASS()
class ARPGAI_API UEnvQueryTest_Safety : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_Safety();
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> TraceChannel_ObstacleOverlap = ECC_Visibility;
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> TraceChannel_Probe = ECC_Visibility;
};