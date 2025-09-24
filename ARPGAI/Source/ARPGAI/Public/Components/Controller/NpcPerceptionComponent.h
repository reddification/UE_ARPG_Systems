#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CombatEvaluationData.h"
#include "Perception/AIPerceptionComponent.h"
#include "NpcPerceptionComponent.generated.h"

struct FCharacterPerceptionData
{
	float Distance = 0.f;
	float Strength = 0.f;
	float TimeSeen = 0.f;
	float Health = 0.f;
	float AccumulatedDamage = 0.f;
	float ForwardVectorsDotProduct = 1.f;
	float DirectionToForwardVectorDotProduct = 1.f;
	FGameplayTag NpcId;
	FGameplayTag Attitude;
	FGameplayTagContainer NpcTags;
	FGameplayTagContainer ProducedSounds;
	NpcCombatEvaluation::EDetectionSource DetectionSource;
	bool bCharacterSeesNpc = false;
	bool bAlly = false;

	bool IsAlive() const { return Health > 0.f; }
	bool IsHostile() const;
};

struct FHeardSounds
{
	FVector Location;
	float Distance = 0.f;
	FGameplayTag SoundTag;
	float Age = 0.f;
	bool bByAlly = false;
};

struct FHazardPerceptionData
{
	FVector Location;
	float Radius = 0.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcPerceptionComponent : public UAIPerceptionComponent
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FTargetPerceptionUpdatedNativeDelegate, AActor* TriggerActor, const FAIStimulus& Stimulus);
	
public:
	UNpcPerceptionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float GetAccumulatedTimeSeen(AActor* Actor) const;

	UFUNCTION(BlueprintCallable)
	float GetAccumulatedDamage() const;
	
	const TMap<TWeakObjectPtr<AActor>, FCharacterPerceptionData>& GetAnimatePerceptionData() { return CharacterPerceptionCache; };
	const TMap<TWeakObjectPtr<AActor>, FHeardSounds>& GetHeardSounds() const { return HeardSoundsCache; };

	FTargetPerceptionUpdatedNativeDelegate TargetPerceptionUpdatedNativeEvent;

	void SetPawn(APawn* InPawn);
	
protected:
	virtual void BeginPlay() override;
	virtual bool ConditionallyStoreSuccessfulStimulus(FAIStimulus& StimulusStore, const FAIStimulus& NewStimulus) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AllyPerceptionMergeDistanceThreshold = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PerceptionCacheInterval = 0.2f;
	
private:
	void CachePerception();
	void CachePerception(const FVector& NpcLocation, const TArray<APawn*>& Allies);
	void MergeAllyPerceptions(const FVector& NpcLocation, const TArray<APawn*>& Allies);
	bool CanMergePerception(APawn* Ally);

	FTimerHandle PerceptionCacheTimer;

	UFUNCTION()
	void OnTargetPerceptionInfoUpdatedHandler(const FActorPerceptionUpdateInfo& UpdateInfo);

	UFUNCTION()
	void OnTargetPerceptionForgottenHandler(AActor* Actor);

	UFUNCTION()
	void OnTargetPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus);

	void OnNpcOwnerDied(AActor* Actor);
	
	TWeakObjectPtr<APawn> OwnerPawn;
	TWeakObjectPtr<UNpcAttitudesComponent> NpcAttitudesComponent;
	
	TMap<TWeakObjectPtr<AActor>, float> ActorsObservationTime;
	
	TMap<TWeakObjectPtr<AActor>, FCharacterPerceptionData> CharacterPerceptionCache;
	TMap<TWeakObjectPtr<AActor>, FHeardSounds> HeardSoundsCache;
	TArray<FHazardPerceptionData> HazardsPerceptionCache;
};
