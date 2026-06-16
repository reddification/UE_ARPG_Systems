#pragma once

#include "CoreMinimal.h"
#include "DebugDataTypes.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcCombatTypes.h"

#include "NpcCombatLogicComponent.generated.h"

struct FNpcDeathEventData;
class AAIController;
class INpcCombatInterface;
struct FNpcFeintParameters;
class UNpcBlackboardDataAsset;
class UNpcCombatParametersDataAsset;
class UNpcCombatAttributeSet;
struct FOnAttributeChangeData;
class UAttributeSet;
class UAbilitySystemComponent;
class INpcAliveActor;
class INpc;

/**
 * 10 May 2026 (aki): TODO refactor this component
 * At this point it does too much. It
 * 1. Handles owner NPC combat attributes
 * 2. Tracks active enemies
 * 3. Reacts to melee attack situations (missed, whiffed, attack incoming)
 * 4. Has mid-term memory (not long not short) of enemies
 * 5. Maintains behavior evaluator combat state (tag, duration)
 * I think it's at least 2 components: 1, 2+3, 4+5 
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcCombatLogicComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcCombatLogicComponent();
	
	FORCEINLINE float GetAttackRange() const { return AttackRange; }
	FORCEINLINE float GetSurroundRange() const { return SurroundRange; }
	FORCEINLINE float GetAggression() const { return Aggressiveness; }
	FORCEINLINE float GetPoise() const { return Poise; }
	FORCEINLINE float GetIntelligence() const { return Intelligence; }
	FORCEINLINE float GetReaction() const { return Reaction; }
	FORCEINLINE float GetNormalizedStamina() const { return NormalizedStamina; }
	FORCEINLINE const FNpcPrimaryCombatTargetData& GetPrimaryTargetData() const { return PrimaryTargetData; }
	FORCEINLINE const AActor* GetPrimaryTargetActor() const { return PrimaryTargetData.Actor.Get(); }
	FORCEINLINE void SetBrainPaused(bool bPaused) { bBrainPaused = bPaused; }
	
	FORCEINLINE const UNpcCombatParametersDataAsset* GetNpcCombatParameters() const { return NpcCombatParameters; }
	
	bool IsRetreating() const;
	bool IsSurrounding() const;
	void SetCombatRole(ENpcCombatRole NpcAttackRole);

	const FNpcFeintParameters& GetAttackFeintParameters() const;
	float GetIntellectAffectedDistance(float BaseDistance) const;
	float GetTauntProbabilityOnSuccessfulAttack() const;
	float GetBackdashProbabilityOnWhiff() const;

	void UpdateImmediateThreats(const FNpcCurrentCombatThreatsContainer& EvaluatedThreats);
	const FNpcCurrentCombatThreatsContainer& GetActiveThreats() const;

	virtual void SetCurrentCombatTarget(AActor* Target, const FGameplayTag& BehaviorTypeTag);
	virtual void ClearCurrentCombatTarget(const FGameplayTag& BehaviorId);
	
	void OnBlockCompleted();
	void OnDodgeCompleted();

	void UpdateBlackboardKeys();
	
	// Set from the BTService, so no need to update in blackboard
	UFUNCTION(BlueprintCallable)
	void SetDistanceToTarget(float NewDistance);
	
	UFUNCTION(BlueprintCallable)
	float GetDistanceToTarget() const { return DistanceToTarget; }

	void SetEvaluatedTargetMoveDirection(ENpcTargetDistanceEvaluation NewTargetMoveDirectionEvaluation);
	ENpcTargetDistanceEvaluation GetTargetMoveDirectionEvaluation() const { return TargetMoveDirectionEvaluation; }
	
	TArray<APawn*> GetAllies(bool bIgnoreSquadLeader) const;

	bool ShouldRetaliateAfterSuccessfulBlock(ENpcBlockResult BlockResult);
	
	bool IsReactingToIncomingAttack() const { return IsValid(DefensiveActionCauser); }
	void ReactToReceivedHit(const FGameplayTag& HitTypeTag, AActor* HitCauser, float HealthDamage, const FHitResult& HitResult);
	
	// (aki) 02.02.2026: used for investigation of a bug. TODO remove ASAP
	virtual void Debug_RequestDodge();
	
	bool DecideWantToBaitAttack();
	float GetBaitAttackDuration() const;
	void ClearEnemiesData();
	
	void OnCombatBehaviorStarted();
	void OnCombatBehaviorEnded();
	
	void UpdateCombatMemory(const TMap<TWeakObjectPtr<AActor>, FNpcEnemyCombatMemory>& NewEnemiesData);
	const TMap<TWeakObjectPtr<AActor>, FNpcEnemyCombatMemory>& GetCombatEnemiesMemory() const { return EnemiesCombatMemory; }
	bool HasTarget(const AActor* Actor) const { return ImmediateThreats.Contains(Actor); }
	const FNpcEnemyCombatMemory* GetActorCombatMemoryData(const AActor* Actor) const;
	void OnDealtDamage(AActor* Actor, float ResultingDamage);

protected:
	FNpcPrimaryCombatTargetData PrimaryTargetData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDebugOptionsContainer DebugOptions;
#endif
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitializeComponent() override;

	UPROPERTY()
	TScriptInterface<INpc> OwnerNPC;

	UPROPERTY()
	TScriptInterface<class INpcActorTagsInterface> OwnerTagsActorNPC;
	
	UPROPERTY()
	TScriptInterface<INpcAliveActor> OwnerAliveCreature;

	UPROPERTY()
	TScriptInterface<INpcCombatInterface> OwnerNpcCombatInterface;
	
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double OutOfCombatEnemiesMemoryStorageDuration = 10.;
	
private:
	bool bNpcComponentInitialized = false;
	bool bDead = false;
	bool bBrainPaused = false;
	
	float AttackRange = 0.f;
	float SurroundRange = 0.f;
	float Aggressiveness = 0.f;
	float Poise = 0.f;

	float Intelligence = 0.f;
	mutable float DistanceIntelligenceDependencyFactorCachedIntelligence = 0.f;
	mutable float DistanceIntelligenceDependencyFactor = 0.f;

	float Reaction = 0.f;
	float Health = 0.f;
	float NormalizedHealth = 0.f;
	float Stamina = 0.f;
	float NormalizedStamina = 0.f;

	float DistanceToTarget = 0.f;
	float AttackRangeScale = 1.f;
	float AttackRangeStepExtension = 80.f;
	float BaitAttackAvailableAtWorldTime = 0.f;

	double LastCombatEndTime = 0.;

	FNpcCurrentCombatThreatsContainer ImmediateThreats;
	TMap<TWeakObjectPtr<AActor>, FNpcEnemyCombatMemory> EnemiesCombatMemory;

	ENpcTargetDistanceEvaluation TargetMoveDirectionEvaluation = ENpcTargetDistanceEvaluation::TargetIsStationary;
	ENpcCombatRole ActiveAttackerRole = ENpcCombatRole::None;
	
	UPROPERTY()
	mutable TObjectPtr<UBlackboardComponent> BlackboardComponent = nullptr;
	
	UPROPERTY()
	TObjectPtr<const AActor> DefensiveActionCauser = nullptr;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AttackingThreats;
	
	UPROPERTY()
	const UNpcCombatParametersDataAsset* NpcCombatParameters = nullptr;
	
	UPROPERTY()
	const UNpcBlackboardDataAsset* NpcBlackboardKeys = nullptr;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerASC = nullptr;
	
	void InitializeCombatData();
	
	void OnAttributeAdded(UAbilitySystemComponent* ASC, const UAttributeSet* Attributes);
	void OnAttackRangeChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnAggressivenessChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnStaminaChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	void OnIntellectChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnReactionChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void InitializeNpcCombatAttributeSet(const UNpcCombatAttributeSet* NpcCombatAttributeSet);

	void ReactToIncomingAttack(AActor* Actor);
	void ReactToThreatAttackCompleted(AActor* Actor);
	void ReactToFeintedAttack(AActor* Attacker);
	void ReactToEnemyWhiffedAttack(AActor* Actor);
	void ReactToEnemyBlock(AActor* Actor);
	void ReactToEnemyChangeWeapon(AActor* Actor);
	void ResetReactionToIncomingAttack();
	void GetRelevantThreatsCounts(int& RelevantEnemiesCount, int& EnemiesThatCanAttackMeCount, float RelevantDistanceScale = 1.25f, float Angle = 30.f) const;
	
	void SetAttackRange(float NewValue);
	void SetSurroundRange(float NewValue);
	void SetAggression(float NewAggression);
	void SetIntelligence(float NewIntelligence);
	void SetReaction(float NewValue);
	void SetHealth(float NewValue);
	void SetStamina(float NewValue);
	void UpdateAttackRangeInBlackboard();
	
	void OnNpcDeathStarted(AActor* OwningActor, const FNpcDeathEventData& DeathEventData);
	void InitializeNpcCombatLogic(AAIController& AIController);

	void UnsubscribeFromDelegates();
	
	bool IsImmobilized() const { return bBrainPaused || bDead; }
};