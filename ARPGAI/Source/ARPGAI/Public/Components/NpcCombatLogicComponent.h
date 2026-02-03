// 

#pragma once

#include "CoreMinimal.h"
#include "EnemiesCoordinatorComponent.h"
#include "GameplayTagContainer.h"
#include "Data/CombatEvaluationData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Tasks/Combat/BTTask_AttackSequence.h"
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
	UNpcCombatLogicComponent();
	
	FORCEINLINE float GetAttackRange() const { return AttackRange; }
	FORCEINLINE float GetSurroundRange() const { return SurroundRange; }
	FORCEINLINE float GetAggression() const { return Aggressiveness; }
	FORCEINLINE float GetPoise() const { return Poise; }
	FORCEINLINE float GetIntelligence() const { return Intelligence; }
	FORCEINLINE float GetReaction() const { return Reaction; }
	FORCEINLINE FNpcDTR* GetNpcDTR() const { return NpcDTRH.GetRow<FNpcDTR>(""); }
	FORCEINLINE const FNpcActiveTargetData& GetPrimaryTargetData() const { return PrimaryTargetData; };
	FORCEINLINE const AActor* GetPrimaryTargetActor() const { return PrimaryTargetData.ActiveTarget.Get(); }
	
	FORCEINLINE const UNpcCombatParametersDataAsset* GetNpcCombatParameters() const { return NpcCombatParameters; };
	
	bool IsRetreating() const;
	bool IsSurrounding() const;
	void SetCombatRole(ENpcCombatRole NpcAttackRole);

	float GetCombatEvaluatorInterval() const;
	const FNpcFeintParameters& GetAttackFeintParameters() const;
	float GetIntellectAffectedDistance(float BaseDistance) const;
	float GetTauntProbabilityOnSuccessfulAttack() const;
	float GetBackstepProbabilityOnWhiff() const;
	
	void SetActiveThreats(const FNpcActiveThreatsContainer& EvaluatedThreats);
	const FNpcActiveThreatsContainer& GetActiveThreats() const;
	FGameplayTag GetThreatLevel(float BestTargetThreat) const;
	void SetEnemyThreatLevel(const FGameplayTag& InThreatLevelTag);

	virtual void SetCurrentCombatTarget(AActor* Target, const FGameplayTag& BehaviorTypeTag);
	virtual void SetCurrentCombatTarget(AActor* Target, const FGameplayTag& BehaviorTypeTag, const FNpcCombatPerceptionData& MobCombatPerceptionData);
	virtual void ClearCurrentCombatTarget();
	
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
	void TrackEnemyAlive(AActor* Actor);
	void ResetTrackingEnemyAlive();
	
	// (aki) 02.02.2026: used for investigation of a bug. TODO remove ASAP
	virtual void Debug_RequestDodge();

protected:
	FNpcActiveTargetData PrimaryTargetData;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitializeComponent() override;

private:
	FDataTableRowHandle NpcDTRH;
	bool bNpcComponentInitialized = false;

	float AttackRange = 0.f;
	float SurroundRange = 0.f;
	float Aggressiveness = 0.f;
	float Poise = 0.f;

	float Intelligence = 0.f;
	mutable float DistanceIntelligenceDependencyFactorCachedIntelligence = 0.f;
	mutable float DistanceIntelligenceDependencyFactor = 0.f;

	float Reaction = 0.f;
	float Health = 0.f;
	float Stamina = 0.f;
	float CombatEvaluatorInterval = 1.0f;
	float Anxiety = 0.f;

	// think about removing it and using Attribute instead 
	float DistanceToTarget = 0.f;
	ENpcTargetDistanceEvaluation TargetMoveDirectionEvaluation = ENpcTargetDistanceEvaluation::TargetIsStationary;
	
	float AttackRangeScale = 1.f;
	float AttackRangeStepExtension = 80.f;

	UPROPERTY()
	TScriptInterface<INpc> OwnerNPC;

	UPROPERTY()
	TScriptInterface<INpcAliveCreature> OwnerAliveCreature;
	
	TWeakObjectPtr<APawn> OwnerPawn;

	mutable TWeakObjectPtr<UBlackboardComponent> BlackboardComponent;
	
	ENpcCombatRole ActiveAttackerRole = ENpcCombatRole::None;
	FGameplayTag ActiveThreatLevelTag = FGameplayTag::EmptyTag;
	TWeakObjectPtr<const AActor> ActiveReactToAttackActor = nullptr;
	FNpcActiveThreatsContainer ActiveThreats;
	
	TWeakObjectPtr<AActor> TrackedEnemyAlive;
	
	TMap<TWeakObjectPtr<const AActor>, float> IgnoreIncomingAttackUntil;

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
	void UpdateAttackRangeInBlackboard();
	
	void AddIgnoredIncomingAttackFromThreat(const AActor* Actor, float TimeToIgnore);
	bool HasIgnoredIncomingAttackFromThreat(const AActor* Actor);
	
	void OnNpcDeathStarted(AActor* OwningActor);
	void InitializeNpcCombatLogic(AAIController& AIController);

	void UnsubscribeFromDelegates();
	void OnEnemyDied(AActor* Actor);
	void ReceiveReportEnemyDied(AActor* KilledActor);
	
	UPROPERTY()
	const UNpcCombatParametersDataAsset* NpcCombatParameters = nullptr;
	
	UPROPERTY()
	const UNpcBlackboardDataAsset* NpcBlackboardKeys = nullptr;
	
	TWeakObjectPtr<UAbilitySystemComponent> OwnerASC;
};