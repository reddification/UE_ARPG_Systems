// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UObject/Object.h"
#include "BTService_CheckIsTargetInLoS.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_CheckIsTargetInLoS : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CheckIsTargetInLoS();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutHasLoSBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 1.f, ClampMax = 1.f))
	float OwnerToTargetDotProductThreshold = 0.75f;
};
