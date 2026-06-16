#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/NpcMemoryDataTypes.h"
#include "Interfaces/NpcAliveActor.h"
#include "NpcMemoryComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcMemoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	
	struct FTraitsMemory
	{
		void AddTemporalTraits(const FGameplayTagContainer& Traits, const FDateTime& UntilGameTime);
		void AddPersistentTraits(const FGameplayTagContainer& Traits);
		void RemoveTraits(const FGameplayTagContainer& RemovedTraits);
		
		FGameplayTagContainer GetTraits() const;
		bool IsEmpty() const { return PersistentTraits.IsEmpty() && TemporalTraits.IsEmpty(); }

		void CleanupStaleTraits(const FDateTime& GameTimeNow);
		bool HasTemporalTraits() const { return !TemporalTraits.IsEmpty(); }
		bool HasTraits() const { return !TemporalTraits.IsEmpty() || !PersistentTraits.IsEmpty(); }

	private:
		FGameplayTagContainer PersistentTraits;
		TMap<FGameplayTag, FDateTime, TInlineSetAllocator<4>> TemporalTraits;
	};

	struct FLongTermMemoryActorData
	{
		FVector LastSeenLocation = FAISystem::InvalidLocation;
		double LastUpdateTime = 0.f;
		FDateTime LastUpdateGameTime = FDateTime();
		FTraitsMemory RememberedTraits;
		FGameplayTag Attitude;
		EDetectionSource LastDetectionSource = EDetectionSource::None;
		bool bAlive = true;
	};

	struct FKilledActorData
	{
		FGuid KilledActorId;
		TWeakObjectPtr<AActor> KilledActor;
		
		FDateTime KilledAt;
		FGameplayTag LastHitTag;
	};
	
	struct FKilledAllyData : public FKilledActorData
	{
		FGuid MurdererId;
		TWeakObjectPtr<AActor> MurderActor;
	};

	friend class UNpcPerceptionComponent;
	
public:
	// Sets default values for this component's properties
	UNpcMemoryComponent();

	void SetPawn(APawn* InPawn);
	
	void RememberLongTerm(AActor* Actor, const FCharacterShortTermMemory& CharacterSTM, const FGuid& Id);
	void CleanupStaleTraitsMemory();
	void RememberActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits);
	void RememberActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits, float ForDurationGTH);
	void ForgetActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits);
	FGameplayTagContainer GetRememberedActorTraits(const AActor* Actor) const;
	
	// GTH threshold <= 0 means no limit
	int GetAlliesKilledByCount(const AActor* Murderer, float GameTimeHoursThreshold);
	void RememberAllyDied(APawn* DeadAlly, AActor* Murderer, const FGameplayTag& LastHitType);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FNpcLongTermMemoryReason> LongTermMemoryReasons;

private:
	void OnNpcTagsChanged(AActor* Pawn, const FGameplayTagContainer& NewTags);
	void RememberTemporaryTrait(const AActor* ForActor, FLongTermMemoryActorData& Memory, const FGameplayTagContainer& Traits, float DurationGTH);
	
	TMap<FGuid, FLongTermMemoryActorData> LongTermMemory;
	
	// TArray<FKilledActorData> KillHistory; // unused for now
	TArray<FKilledAllyData> DeadAlliesHistory;
	
	FTimerHandle CleanUpRememberedTraitsTimer;
	FGameplayTagContainer OwnerNpcTags;
	
	UPROPERTY()
	APawn* OwnerPawn = nullptr;
};
