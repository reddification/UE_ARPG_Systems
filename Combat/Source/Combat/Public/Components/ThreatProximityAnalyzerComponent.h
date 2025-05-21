// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThreatProximityAnalyzerComponent.generated.h"


class ICombatant;
class ICombatAnimInstance;

COMBAT_API DECLARE_LOG_CATEGORY_EXTERN(LogCombat_ThreatProximity, Verbose, All);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UThreatProximityAnalyzerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UThreatProximityAnalyzerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ConsideredCloseCombatRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float SwitchToThreatIsFarDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 1.f, ClampMax = 1.f))
	float DotProductThreshold = 0.75f;

private:
	float ConsideredCloseCombatRangeSq = 500.f * 500.f;
	float RemainingDelaySwitchToThreatIsFar = 0.f;
	
	TScriptInterface<ICombatant> CombatantOwner;
	TScriptInterface<ICombatAnimInstance> CombatAnimInstance = nullptr;

	void OnOwnerDied(AActor* Actor);
};
