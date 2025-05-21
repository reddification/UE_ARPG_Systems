// 

#pragma once

#include "CoreMinimal.h"
#include "MeleeBlockComponent.h"
#include "PlayerManualBlockComponent.generated.h"

class IPlayerCombatant;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UPlayerManualBlockComponent : public UMeleeBlockComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerManualBlockComponent();
	virtual void StartBlocking() override;
	virtual void StopBlocking() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual FVector2D GetBlockInput(float DeltaTime) const override;
	
private:
	UPROPERTY()
	TScriptInterface<IPlayerCombatant> OwnerPlayerCombat;
	
	float PlayerBlockInputAccumulationScale = 1.5f;
	TWeakObjectPtr<AActor> FocusTarget = nullptr;
	double LastDistSqBetweenFocusedEnemyAndOwner = -FLT_MAX;
	double DistSqToUpdateFocusedEnemy = 50.f * 50.f;
	
	void UpdateFocus();
};
