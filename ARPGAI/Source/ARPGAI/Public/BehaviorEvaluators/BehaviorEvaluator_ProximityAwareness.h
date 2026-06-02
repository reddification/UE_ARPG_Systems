#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "AITypes.h"
#include "Components/NpcProximityAwarenessComponent.h"
#include "BehaviorEvaluator_ProximityAwareness.generated.h"

UCLASS(DisplayName="Beast | Proximity awareness")
class ARPGAI_API UBehaviorEvaluatorConfig_ProximityAwareness : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_ProximityAwareness();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WakeUpToNoiseChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AwarenessLocationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> WakeUpToNoises;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery DetectionBlockedFilter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceScoreDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve TimePerceivedScoreDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Radius = 400.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ProximityAwarenessUpdateInterval = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TEnumAsByte<ECollisionChannel>> ObjectTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIgnoreAllies = true;

	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_ProximityAwareness : public FBehaviorEvaluator_Base
{
private:
	using Super = FBehaviorEvaluator_Base;

	struct FProximityAwarenessCandidate
	{
		FProximityAwarenessCandidate() {  }
		FProximityAwarenessCandidate(float Score, const AActor* Actor) : Instigator(Actor), Location(Actor->GetActorLocation()), Score(Score) {  }
		FProximityAwarenessCandidate(float Score, const FVector& Location) : Location(Location), Score(Score) {  }
		
		TWeakObjectPtr<const AActor> Instigator = nullptr;
		FVector Location = FAISystem::InvalidLocation;
		float Score = 0.f;
		
		bool operator < (const FProximityAwarenessCandidate& Other) const
		{
			const float MyActorScale = Instigator.IsValid() ? 1.5f : 1.f;
			const float OtherActorScale = Other.Instigator.IsValid() ? 1.5f : 1.f; 
			return Score * MyActorScale > Other.Score * OtherActorScale;
		}
	};
	
public:
	FBehaviorEvaluator_ProximityAwareness(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	virtual void Update(const float DeltaTime) override;
	virtual void SetState(EBehaviorEvaluatorState NewState) override;
	
protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;

private:
	FGameplayTag GetMatchingSoundTag(const FGameplayTag& SoundTag) const;
	
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_ProximityAwareness> ProximityAwarenessConfig;
	TWeakObjectPtr<UNpcProximityAwarenessComponent> ProximityAwarenessComponent;
	FProximityAwarenessCandidate ActiveAwarenessCandidate;
	FGameplayTagContainer AttractingSoundsCache;
};
