#pragma once

#include "CoreMinimal.h"
#include "MeleeBlockComponent.h"
#include "NpcBlockComponent.generated.h"


class INpcCombatant;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UNpcBlockComponent : public UMeleeBlockComponent
{
	GENERATED_BODY()

public:
	void StartBlocking(const AActor* AttackingActor, EMeleeAttackType IncomingAttackType);
	void StartGuidedBlocking();

	virtual void StartBlocking() override;
	virtual void StopBlocking() override;
	
	FORCEINLINE void SetExternalBlockInput(const FVector2D& NewExternalInput) { ExternalBlockInput = NewExternalInput; }
	
	mutable FSimpleDelegate OnNpcFinishedBlockingEvent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual FVector2D GetBlockInput(float DeltaTime) const override;
	virtual void AddBlockInput(const FVector2D& BlockDirectionInput, float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NpcBlockDrawRateScale = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double ReactionDelayScaleMax = 0.3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double ReactionDelayScaleMin = 1.25;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NonParryingBlockStrengthAccumulationScale = 0.15f;
	
private:
	void SetBlockReactionDelay();
	void ReleaseBlock();
	
	TArray<FVector2D> PendingBlockInputs;
	FVector2D CurrentAccumulatedBlock = FVector2D::ZeroVector;
	FVector2D ExternalBlockInput = FVector2D::ZeroVector;
	int CurrentPendingBlockIndex = 0;

	bool bUsingExternalBlockInputSource = false;
	float NpcHoldBlockDurationMin = 0.2f;
	float NpcHoldBlockDurationMax = 0.5f;
	float InitialBlockStrengthAccumulationScale = 0.5f;
	FTimerHandle ReleaseBlockTimer;
	FTimerHandle StartBlockReactionDelayTimer;
	bool bNpcBlockOnHold = false;

	UPROPERTY()
	TScriptInterface<INpcCombatant> NpcCombatant;
	
	void Debug_TriggerHitReact();
};
