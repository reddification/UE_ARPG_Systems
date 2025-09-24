// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_TagCooldown.h"
#include "BTDecorator_SquadTagCooldown.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_SquadTagCooldown : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_SquadTagCooldown(const FObjectInitializer& ObjectInitializer);
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	void SetCooldownForSquadMembers(const UBehaviorTreeComponent& BehaviorComp) const;
	
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	UPROPERTY(EditAnywhere)
	bool bOnNodeActivation = true;

	UPROPERTY(EditAnywhere)
	bool bOnNodeDeactivation = false;

	UPROPERTY(EditAnywhere)
	bool bActivateCooldownToSelfOnDeactivation = true;

	UPROPERTY(EditAnywhere)
	bool bLimitByRange = true;

	UPROPERTY(EditAnywhere)
	FGameplayTag CooldownTag;

	UPROPERTY(EditAnywhere)
	FValueOrBBKey_Float CooldownDuration = 5.f;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition = bLimitByRange, UIMin = 0.f, ClampMin = 0.f))
	float DistanceToAllyThreshold = 750.f;
};
