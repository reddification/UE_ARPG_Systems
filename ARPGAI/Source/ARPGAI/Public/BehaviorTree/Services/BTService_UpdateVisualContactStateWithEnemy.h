// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateVisualContactStateWithEnemy.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_UpdateVisualContactStateWithEnemy : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTMemory_UpdateVisualContactState
	{
		FVector LastSeenLocation = FAISystem::InvalidLocation;
	};
	
public:
	UBTService_UpdateVisualContactStateWithEnemy();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_UpdateVisualContactState); };
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutNpcSeesEnemyBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutEnemySeesNpcBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutDotProduct_NpcFV_EnemyFV_BBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutVisualContactDurationBBKey;
	
	// dot product between enemy forward vector and NPC forward vector to understand if NPC is in enemy`s LoS
	// -1 is they are staring direct at each other, the bigger the value - the less NPC is in enemy`s LoS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 0.f, ClampMax = 0.f))
	float NpcFVToEnemyFVDotProductThreshold = -0.88f;

	// NPC is only considered to see enemy when dot product between NPC's forward vector and vector from NPC to enemy is bigger (or equal) than this value
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 1.f, ClampMax = 1.f))
	float NpcSeesEnemyDotProductThreshold = 0.75f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.1f, ClampMin = 0.1f))
	float VisualContactTimerDecayRate = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.0f, ClampMin = 0.0f, UIMax = 1.f, ClampMax = 1.f))
	float ChanceToReportVisualContactAcquired = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.0f, ClampMin = 0.0f, UIMax = 1.f, ClampMax = 1.f))
	float ChanceToReportVisualContactLost = 0.25f;

	// When NPC loses direct sight contact, keep state of Npc sees enemy for this duration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.0f, ClampMin = 0.0f))
	float SightLostDelay = 2.f;

private:
	void ReportVisualContactStateChanged(UBehaviorTreeComponent& OwnerComp, APawn* NpcPawn,
	                                     float ChanceToReport, const FGameplayTag& ReportTag);
};
