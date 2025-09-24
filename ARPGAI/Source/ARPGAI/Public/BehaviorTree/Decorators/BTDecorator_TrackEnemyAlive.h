// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_TrackEnemyAlive.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_TrackEnemyAlive : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_TrackEnemyAlive : public FBTAuxiliaryMemory
	{
		FDelegateHandle BlackboardObserverDelegateHandle;
	};
	
public:
	UBTDecorator_TrackEnemyAlive();
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_TrackEnemyAlive); };
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ActiveTargetBBKey;

private:
	EBlackboardNotificationResult OnEnemyChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};
