// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTargetWithinNpcArea.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_UpdateTargetWithinNpcArea : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateTargetWithinNpcArea();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutIsTargetOutOfNpcAreaBBKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetActorBBKey;

	// secondary way to check location in case target actor is nullptr
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector PredictedTargetLocationBBKey;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	float AreaExtent = 300.f;
};
