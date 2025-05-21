// Copyright Pixagon Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_AttackCoordinator.generated.h"

class UEQSContext_FocusActor;
/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryTest_AttackCoordinator : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_AttackCoordinator(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionDetails() const override;

	/** context */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UEQSContext_FocusActor> BotTargetContext;
};
