// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BTService.h"
#include "BTService_MultiFocus.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_MultiFocus : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_MultiFocus();

	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	virtual FString GetStaticDescription() const override;

	EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

protected:
	// The higher the index - the more prioritized point is
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBlackboardKeySelector> PrioritizedFocusPointsBBKeys;

private:
	const EAIFocusPriority::Type BasePriority = EAIFocusPriority::Gameplay;
};
