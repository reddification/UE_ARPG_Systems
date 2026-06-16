#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "AITypes.h"
#include "Data/NpcMemoryDataTypes.h"
#include "Operations/BehaviorEvaluatorOperations_DataTypes.h"
#include "BehaviorEvaluator_Investigate.generated.h"

USTRUCT(BlueprintType)
struct FSoundEventTraitInvestigationScale
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Scale = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPerceivedSoundTrait TestTrait = EPerceivedSoundTrait::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCheckPresent = true;
};

USTRUCT(BlueprintType)
struct FSoundInvestigationParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Score = 1.f;
	
	// used only for sorting multiple investigation causes so that investigation utility doesnt explode 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PriorityScale = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSoundEventTraitInvestigationScale> TraitScales;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Can Ignore")
	bool bCanIgnore = true;	
	
	// after finished investigating sounds, ignore next appearance for this time duration in game time hours 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bCanIgnore"), Category="Can Ignore")
	float IgnoreDurationGTH = 0.2f; 	
	
	// after finished investigating sounds, ignore next appearance in radius
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bCanIgnore"), Category="Can Ignore")
	float IgnoreRadius = 300.f; 	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bCanIgnore"), Category="Can Ignore")
	FGameplayTagContainer IgnoreWhenOwnerHasTags; 	
};

UCLASS(DisplayName="Investigate")
class ARPGAI_API UBehaviorEvaluatorConfig_Investigate : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_Investigate();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Output")
	FBlackboardKeySelector InvestigateLocationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Output")
	FBlackboardKeySelector InvestigateActorBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Output")
	FBlackboardKeySelector InvestigationDurationBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Output")
	FBlackboardKeySelector InvestigationTagsBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters|Urgency")
	FGameplayTagContainer UrgentInvestigationReasons;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters|Urgency")
	FGameplayTag UrgentInvestigationStateTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
	FRuntimeFloatCurve AttractingSoundToDistanceDependencyCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
	FRuntimeFloatCurve InterestingActorDistanceDependencyCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
	TMap<FGameplayTag, FSoundInvestigationParams> AttractingSoundsOptions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
	TArray<FActorInterestCondition> ActorInterests;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
	FFloatInterval InvestigationDurationInterval;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters", meta=(UMin = 0.f, ClampMin = 0.f))
	float ActiveInvestigationPrioritizationScale = 1.25f;

	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Investigate : public FBehaviorEvaluator_Base
{
	
private:
	using Super = FBehaviorEvaluator_Base;

	struct FInvestigationCandidate
	{
		enum class EInvestigationCause : uint8
		{
			None,
			Sound,
			Actor
		};
		
		FInvestigationCandidate() {  }
		FInvestigationCandidate(float InScore, float InPriorityScale, const FVector& InLocation, AActor* Causer, const FGameplayTag& InEventTag) 
			: Score(InScore), PriorityScale(InPriorityScale), Location(InLocation), EventTag(InEventTag), Actor(Causer),
		InvestigationCause(EInvestigationCause::Sound) { }
		
		FInvestigationCandidate(float InScore, float InPriorityScale, AActor* InActor, const FGameplayTag& InterestCauseTag)
			: Score(InScore), PriorityScale(InPriorityScale), Location(InActor->GetActorLocation()),
			EventTag(InterestCauseTag), Actor(InActor), InvestigationCause(EInvestigationCause::Actor) { };
		
		float Score = 0.f;
		float PriorityScale = 0.f;
		FVector Location = FAISystem::InvalidLocation;
		FGameplayTag EventTag;
		TWeakObjectPtr<AActor> Actor = nullptr;
		EInvestigationCause InvestigationCause = EInvestigationCause::None;
		bool bUrgent = false;

		bool operator < (const FInvestigationCandidate& Other) const
		{
			return Score * PriorityScale > Other.Score * Other.PriorityScale;
		}

		FString ToString() const;
		bool IsValidActorEvent() const;
		bool IsValidLocationEvent() const;
		bool IsValid() const { return IsValidActorEvent() || IsValidLocationEvent(); };
	};
	
	struct FIgnoredSound
	{
		FIgnoredSound(const FVector& Location, const FDateTime& UntilGameTime) : Location(Location), UntilGameTime(UntilGameTime)
		{
		}

		FVector Location = FAISystem::InvalidLocation;
		FDateTime UntilGameTime = FDateTime();
	};
	
public:
	FBehaviorEvaluator_Investigate(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	virtual void Update(const float DeltaTime) override;
	
protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;
	virtual void HandleMessage_Internal(const FGameplayTag& MessageTag) override;
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Investigate> InvestigateConfig;
	FGameplayTag GetMatchingSoundTag(const FGameplayTag& SoundTag) const;
	bool IsSoundIgnored(const FGameplayTag& SoundTag, const FSoundInvestigationParams& SoundParams, const FVector& TestLocation) const;
	float GetTraitsScale(const TArray<FSoundEventTraitInvestigationScale>& TraitChecks, EPerceivedSoundTrait Traits) const;
	float UpdatePerception();
	
	FInvestigationCandidate ActiveInvestigation;
	TMap<FGameplayTag, TArray<FIgnoredSound>> IgnoredSoundsLocations;
	FGameplayTagContainer AttractingSoundsCache;
	FGameplayTagContainer OwnerTags;
};
