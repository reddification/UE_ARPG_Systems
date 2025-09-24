// 

#pragma once

#include "CoreMinimal.h"
#include "EnemiesCoordinatorComponent.h"
#include "GameplayTagContainer.h"
#include "Data/CombatEvaluationData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcCombatTypes.h"
#include "Data/NpcDTR.h"

#include "NpcCombatLogicComponent.generated.h"


class UNpcCombatAttributeSet;
struct FOnAttributeChangeData;
class UAttributeSet;
class UAbilitySystemComponent;
class INpcAliveCreature;
class INpc;
using namespace NpcCombatEvaluation;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcCombatLogicComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE float GetAttackRange() const { return AttackRange; }
	FORCEINLINE float GetSurroundRange() const { return SurroundRange; }
	FORCEINLINE float GetAggression() const { return Aggressiveness; }
	FORCEINLINE float GetPoise() const { return Poise; }
	FORCEINLINE FNpcDTR* GetNpcDTR() const { return NpcDTRH.GetRow<FNpcDTR>(""); }

	bool TryForgiveReceivingDamage(const APawn* DamageCauser);
	bool IsRetreating() const;
	bool IsSurrounding() const;
	void SetAttackerRole(ENpcSquadRole NpcAttackRole);

	float GetCombatEvaluatorInterval() const;
	const FNpcFeintParameters& GetAttackFeintParameters() const;
	float GetIntellectAffectedDistance(float BaseDistance) const;
	float GetTauntProbabilityOnSuccessfulAttack() const;
	float GetBackstepProbabilityOnWhiff() const;
	
	void SetActiveThreats(const FNpcActiveThreatsContainer& EvaluatedThreats);
	const FNpcActiveThreatsContainer& GetActiveThreats() const;
	FGameplayTag GetThreatLevel(float BestTargetThreat) const;
	void SetEnemyThreatLevel(const FGameplayTag& InThreatLevelTag);

	virtual void SetCurrentCombatTarget(AActor* Target, const FNpcCombatPerceptionData& MobCombatPerceptionData, const FGameplayTag& BehaviorTypeTag);
	virtual void ResetCurrentCombatTarget();
	const FNpcActiveTargetData& GetCurrentCombatTarget() const;
	
	void OnBlockCompleted();
	void OnDodgeCompleted();

	void UpdateBlackboardKeys();
	
	// Set from the BTService, so no need to update in blackboard
	UFUNCTION(BlueprintCallable)
	void SetDistanceToTarget(float NewDistance) { DistanceToTarget = NewDistance; }
	
	UFUNCTION(BlueprintCallable)
	float GetDistanceToTarget() const { return DistanceToTarget; }

	TArray<APawn*> GetAllies(bool bIgnoreSquadLeader) const;
	void TrackEnemyAlive(AActor* Actor);
	void ResetTrackingEnemyAlive();

protected:
	FNpcActiveTargetData CurrentTargetData;

	void InitializeCombatData();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FDataTableRowHandle NpcDTRH;
	bool bNpcComponentInitialized = false;

	float AttackRange = 0.f;
	float SurroundRange = 0.f;
	float Aggressiveness = 0.f;
	float Poise = 0.f;

	float Intelligence = 0.f;
	mutable float DistanceIntelligenceDependcyFactorCachedIntelligence = 0.f;
	mutable float DistanceIntelligenceDependcyFactor = 0.f;

	float Reaction = 0.f;
	float Health = 0.f;
	float Stamina = 0.f;
	float CombatEvaluatorInterval = 1.0f;
	float Anxiety = 0.f;

	float DistanceToTarget = 0.f;
	float AttackRangeScale = 1.f;

	UPROPERTY()
	TScriptInterface<INpc> OwnerNPC;

	UPROPERTY()
	TScriptInterface<INpcAliveCreature> OwnerAliveCreature;
	
	TWeakObjectPtr<APawn> OwnerPawn;

	mutable TWeakObjectPtr<UBlackboardComponent> BlackboardComponent;
	
	ENpcSquadRole ActiveAttackerRole = ENpcSquadRole::None;
	FGameplayTag ActiveThreatLevelTag = FGameplayTag::EmptyTag;
	TWeakObjectPtr<const AActor> ActiveReactToAttackActor = nullptr;
	FNpcActiveThreatsContainer ActiveThreats;
	TWeakObjectPtr<AActor> TrackedEnemyAlive;

	TMap<TWeakObjectPtr<const AActor>, float> IgnoreIncomingAttackUntil;
	TMap<TWeakObjectPtr<const APawn>, int> ReceivedHitsCountFromCharacters; 
	TMap<FGameplayTag, int> ForgivableCountOfHitsForAttitude;
	
	void OnAttributeAdded(UAbilitySystemComponent* ASC, const UAttributeSet* Attributes);
	void OnAttackRangeChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnAggressivenessChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnStaminaChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	void OnIntellectChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnReactionChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void InitializeNpcCombatAttributeSet(UAbilitySystemComponent* ASC, const UNpcCombatAttributeSet* NpcCombatAttributeSet);

	void ReactToIncomingAttack(AActor* Actor);
	void ReactToFeintedAttack(AActor* Attacker);
	void ReactToEnemyWhiffedAttack(AActor* Actor);
	void ReactToEnemyBlock(AActor* Actor);
	void ReactToEnemyChangeWeapon(AActor* Actor);
	void ResetReactionToIncomingAttack();
	
	void SetAttackRange(float NewValue);
	void SetSurroundRange(float NewValue);
	void SetAggression(float NewAggression);
	void SetIntelligence(float NewIntelligence);
	void SetReaction(float NewValue);
	void SetHealth(float NewValue);
	void SetStamina(float NewValue);
	void SetCombatEvaluatorInterval(float NewValue);
	
	void AddIgnoredIncomingAttackFromThreat(const AActor* Actor, float TimeToIgnore);
	bool HasIgnoredIncomingAttackFromThreat(const AActor* Actor);
	
	void OnNpcDeathStarted(AActor* OwningActor);
	void InitializeNpcCombatLogic(AAIController& AIController);

	void UnsubscribeFromDelegates();
	void OnEnemyDied(AActor* Actor);
	void ReceiveReportEnemyDied(AActor* KilledActor);
};