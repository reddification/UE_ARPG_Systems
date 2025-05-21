// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UObject/Object.h"
#include "BTService_FloatFluctuation.generated.h"

UCLASS()
class ARPGAI_API UBTService_FloatFluctuation : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTMemory_FluctuatingFloat : public FBTAuxiliaryMemory
	{
		float InitialValue = 0.f;
		FDelegateHandle BlackboardObserverHandle;
	};

public:
	UBTService_FloatFluctuation();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_FluctuatingFloat); };
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector FloatBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FluctuationNegative = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FluctuationPositive = 150.f;
	
	// if true, fluctuation would be considered between 0.f and 1.f as a percent
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseFraction = false;

private:
	EBlackboardNotificationResult OnBlackboardKeyChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};
