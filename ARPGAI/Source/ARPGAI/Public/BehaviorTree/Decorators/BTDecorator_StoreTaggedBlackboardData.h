// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_StoreTaggedBlackboardData.generated.h"

class UNpcComponent;

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_StoreTaggedBlackboardData : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_StoreTaggedBlackboardData : public UBTAuxiliaryNode
	{
		FVector PreviousLocation = FAISystem::InvalidLocation;
		TWeakObjectPtr<AActor> PreviousActor = nullptr;
		FDelegateHandle BlackboardDelegateHandle;
	};
	
public:
	UBTDecorator_StoreTaggedBlackboardData();
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_StoreTaggedBlackboardData); };
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	UPROPERTY(EditAnywhere)
	FGameplayTag DataTag;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector DataBBKey;

private:
	EBlackboardNotificationResult OnDataChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	void StoreData(const UBlackboardComponent* Blackboard, UNpcComponent* NpcComponent,
				   FBTMemory_StoreTaggedBlackboardData* BTMemory = nullptr);

};
