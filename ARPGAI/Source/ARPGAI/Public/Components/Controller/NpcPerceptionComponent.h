#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Data/NpcMemoryDataTypes.h"
#include "Interfaces/NpcAliveActor.h"
#include "Perception/AIPerceptionComponent.h"
#include "NpcPerceptionComponent.generated.h"

class UNpcMemoryComponent;
class UNpcAttitudesComponent;
class INpcPerceptionInterface;
class UNpcAreasComponent;
class UNpcCombatSettings;

// 28 Apr 2026 (aki): TODO decouple this component from perception component and rename it to UNpcMemoryComponent
UCLASS(Blueprintable)
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
	float GetAccumulatedDamage(bool bRecalculate) const;
	
	const TMap<TWeakObjectPtr<AActor>, FCharacterShortTermMemory>& GetShortTermCharactersMemory() const { return ShortTermCharacterMemory; }
	const TMap<TWeakObjectPtr<AActor>, TArray<FHeardSoundMemory>>& GetHeardSounds() const { return ShortTermSoundsMemory; }
	const FCharacterShortTermMemory* GetShortTermCharactersMemory(AActor* Actor) const { return ShortTermCharacterMemory.Find(Actor); }
	const TArray<FNpcValueableItemPerceptionData>& GetPerceivedValueableItems() const { return ShortTermValueablesMemory; }
	
	void SetCombatLogicComponent(const UNpcCombatLogicComponent* InCombatLogicComponent);
	void SetMemoryComponent(UNpcMemoryComponent* InMemoryComponent);
	void RememberAllyDied(APawn* DeadAlly, AActor* Murderer, const FGameplayTag& LastHitType);

	mutable FTargetPerceptionUpdatedNativeDelegate TargetPerceptionUpdatedNativeEvent;
	void SetPawn(APawn* InPawn);
	
	void AddExternalVisualPerception(AActor* TriggerActor, float ObservationTime);
	void AddExternalAudioPerception(AActor* TriggerActor, const FGameplayTag& SoundTag, float Loudness, float MaxRange);
	void AddExternalDamagePerception(AActor* TriggerActor, float PerceivedDamage);

protected:
	virtual void BeginPlay() override;
	
	virtual bool ConditionallyStoreSuccessfulStimulus(FAIStimulus& StimulusStore, const FAIStimulus& NewStimulus) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AllyPerceptionMergeDistanceThreshold_VisualContact = 2000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AllyPerceptionMergeDistanceThreshold_Assumption = 600.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PerceptionCacheInterval = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Territory")
	float SoundEventTerritoryBoundsCheckExtent = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Territory")
	float VisibleActorTerritoryBoundsCheckExtent = 250.f;
	
private:
	void ProcessShortTermMemory();
	void PostProcessSoundEvents(const FVector &NpcLocation, const TArray<APawn*>& Allies);
	// bActive means is it a memory of sight perception or does NPC see actor right now like right now-right now
	void CacheVisualPerception(const FVector& NpcLocation, AActor* PerceivedActor, float ObservationTime, bool bAlly, bool bActive);
	void CacheDamagePerception(AActor* PerceivedActor, float ReceivedDamage);
	bool CanResolveHeardUnseenCharacterIdentity(AActor* PerceivedActor, const FGameplayTag& SoundTag) const;
	void RememberHeardSound(const FVector& NpcLocation, AActor* PerceivedActor, const FGameplayTag& SoundTag,
	                        const FVector& SoundLocation, bool bByAlly, float SoundPerceptionAge, float Loudness);
	void ProcessShortTermMemory(const FVector& NpcLocation, const TArray<APawn*>& Allies);
	void MergeAllyPerceptions(const FVector& NpcLocation, const TArray<APawn*>& Allies);
	bool CanMergePerception(APawn* Ally);

	FTimerHandle PerceptionCacheTimer;

	UFUNCTION()
	void OnTargetPerceptionInfoUpdatedHandler(const FActorPerceptionUpdateInfo& UpdateInfo);

	UFUNCTION()
	void OnTargetPerceptionForgottenHandler(AActor* Actor);

	UFUNCTION()
	void OnTargetPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus);
	
	void OnNpcOwnerDied(AActor* Actor, const FNpcDeathEventData& DeathEventData);
	void OnNpcTagsChanged(AActor* Pawn, const FGameplayTagContainer& NewTags);
	
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
	
	UPROPERTY()
	TScriptInterface<INpcPerceptionInterface> NpcPerceptionInterface;
	
	UPROPERTY()
	UNpcAttitudesComponent* NpcAttitudesComponent;
	
	UPROPERTY()
	const UNpcAreasComponent* NpcAreasComponent;
	
	UPROPERTY()
	const UNpcCombatLogicComponent* CombatLogicComponent;
	
	UPROPERTY()
	UNpcMemoryComponent* MemoryComponent;
	
	UPROPERTY()
	TScriptInterface<INpcThreat> OwnerThreat;
	
	TMap<TWeakObjectPtr<AActor>, float> ActorsObservationTime;
	
	TMap<TWeakObjectPtr<AActor>, FCharacterShortTermMemory> ShortTermCharacterMemory;
	TMap<TWeakObjectPtr<AActor>, TArray<FHeardSoundMemory>> ShortTermSoundsMemory;
	TArray<FNpcValueableItemPerceptionData> ShortTermValueablesMemory;
	
	TArray<FHazardPerceptionData> ShortTermHazardsMemory;
	
	FGameplayTagContainer OwnerNpcTags;
	
	TWeakObjectPtr<const UNpcCombatSettings> NpcCombatSettings;
	
	float MyDamageOutput = 0.f;
	float MyProtectionValue = 0.f;
	float MyHearingLoudnessThreshold = 1.f;
	float AccumulatedDamage = 0.f;
};
