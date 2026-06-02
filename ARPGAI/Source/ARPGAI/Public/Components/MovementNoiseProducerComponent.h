#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "MovementNoiseProducerComponent.generated.h"

class INpcMovementNoiseProducer;

UCLASS()
class ARPGAI_API UMovementNoiseProducerConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve SpeedToNoiseDependency;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve CarriedWeightToNoiseDependency;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve LoudnessToSoundRangeDependency;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve DexterityToNoiseDependency;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> SurfaceTypeToNoiseScales;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float AccumulationRate = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float DecayRate = 1.f;

	// noise event reported to AI system when accumulator reaches this value
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float NoiseAccumulationThreshold = 2.f;
};

UCLASS(Blueprintable)
class ARPGAI_API UMovementNoiseProducerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMovementNoiseProducerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMovementNoiseProducerConfig> Config;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bShowDebug = false;

private:
	UPROPERTY()
	TScriptInterface<INpcMovementNoiseProducer> NoiseProducer;
	
	void EmitNoise(float Loudness) const;
	
	float NoiseAccumulator = 0.f;
	FName FootstepTag = NAME_None;
};
