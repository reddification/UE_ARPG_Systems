#pragma once

#include "NpcGoalItem.generated.h"

class UNpcGoalBase;

USTRUCT(BlueprintType)
struct FNpcGoalItem
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	UNpcGoalBase* NpcGoal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRunIndefinitely = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, EditCondition="bRunIndefinitely == false"))
	float GameTimeDurationInHours = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f, EditCondition="bRunIndefinitely == false"))
	float RandomTimeDeviation = 0.2f;

	FString GetDescription() const;
};