// 

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
	void StartBlocking(float Angle);
	
	virtual void StartBlocking() override;
	mutable FSimpleDelegate OnNpcFinishedBlockingEvent;

protected:
	virtual void BeginPlay() override;

	virtual FVector2D GetBlockInput(float DeltaTime) const override;
	virtual void AddBlockInput(const FVector2D& BlockDirectionInput, float DeltaTime) override;
	
private:
	void SetBlockReactionDelay();
	
	TArray<FVector2D> PendingBlockInputs;
	FVector2D CurrentAccumulatedBlock = FVector2D::ZeroVector;
	int CurrentPendingBlockIndex = 0;
	float CurrentReactionDelay = 0.f;
	bool bNpcStartedBlocking = false;

	UPROPERTY()
	TScriptInterface<INpcCombatant> NpcCombatant;
};
