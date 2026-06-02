#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/NpcMemoryDataTypes.h"
#include "NpcMemoryComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcMemoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	
	struct FTraitsMemory
	{
		struct FTemporalTraits
		{
			FGameplayTagContainer Traits {};
			FDateTime UntilGameTime {};
		};
		
		void AddTemporalTraits(const FGameplayTagContainer& Traits, const FDateTime& UntilGameTime);
		void AddPersistentTraits(const FGameplayTagContainer& Traits);
		void RemoveTraits(const FGameplayTagContainer& Traits);
		
		FGameplayTagContainer GetTraits() const;
		bool IsEmpty() const { return PersistentTraits.IsEmpty() && TemporalTraits.IsEmpty(); };

		int CleanupStaleTraits(const FDateTime& GameTimeNow);
		
	private:
		FGameplayTagContainer PersistentTraits;
		TArray<FTemporalTraits, TInlineAllocator<2>> TemporalTraits;
	};

	struct FLongTermMemoryActorData
	{
		FVector LastSeenLocation = FAISystem::InvalidLocation;
		double LastUpdateTime = 0.f;
		FTraitsMemory RememberedTraits;
		FGameplayTagContainer ActorTags;
		FGameplayTag Attitude;
		EDetectionSource LastDetectionSource = EDetectionSource::None;
		bool bAlive = true;
	};

	struct FKilledActorData
	{
		FGuid ActorId;
		TWeakObjectPtr<AActor> Actor;
		FDateTime KilledAt;
		FGameplayTag LastHitTag;
	};

	friend class UNpcPerceptionComponent;
	
public:
	// Sets default values for this component's properties
	UNpcMemoryComponent();

	void SetPawn(APawn* InPawn);
	
	void RememberLongTerm(AActor* Actor, const FCharacterPerceptionData& CharacterPerception, const FGuid& Id);
	void CleanupStaleTraitsMemory();
	void RememberActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits);
	void RememberActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits, float ForDurationGTH);
	void ForgetActorTraits(const AActor* Actor, const FGameplayTagContainer& Traits);
	FGameplayTagContainer GetRememberedActorTraits(const AActor* Actor) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FNpcLongTermMemoryReason> LongTermMemoryReasons;

private:
	FDateTime GetDateTime(float ForDurationGTH = 0.f) const;
	void OnNpcTagsChanged(AActor* Pawn, const FGameplayTagContainer& NewTags);
	
	TMap<FGuid, FLongTermMemoryActorData> LongTermMemory;
	TArray<FKilledActorData> KillHistory;
	FTimerHandle CleanUpRememberedTraitsTimer;
	FGameplayTagContainer OwnerNpcTags;
	
	UPROPERTY()
	APawn* OwnerPawn = nullptr;
};
