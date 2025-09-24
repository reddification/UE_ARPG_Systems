#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_Attack.h"
#include "Data/NpcDTR.h"
#include "GAS/Attributes/NpcCombatAttributeSet.h"
#include "NpcComponent.generated.h"

class INpcZone;
class UBoxComponent;
class INpcAliveCreature;
class INpc;
class INpcControllerInterface;

class UBlackboardComponent;

UCLASS(meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE(FOnDamageReceived)
	DECLARE_MULTICAST_DELEGATE_TwoParams(FNpcCombatStateChanged, const FGameplayTag& NewState, bool bActive);

public:
	UNpcComponent(const FObjectInitializer& ObjectInitializer);
	
	FORCEINLINE bool IsAlive() const { return !bDead; }

	FORCEINLINE void SetDTRH(const FDataTableRowHandle& InNpcDTRH) { NpcDTRH = InNpcDTRH; }
	
	FORCEINLINE const FDataTableRowHandle& GetDTRH() const { return NpcDTRH; }
	
	const FNpcDTR* GetNpcDTR() const;
	
	void OnDamageReceived(float DamageAmount, const FOnAttributeChangeData& ChangeData);

	const FGameplayTag& GetNpcIdTag() const;

	void SetStateActive(const FGameplayTag& StateTag, const TMap<FGameplayTag, float>& SetByCallerParams, bool bInActive);
	
	FGameplayTagContainer GetNpcTags() const;
	const struct FNpcRealtimeDialogueLine* GetDialogueLine(const FGameplayTag& LineTagId) const;

	FORCEINLINE void SetMovementPaceType(const FGameplayTag& NewMovementPaceType) { CurrentMovementPaceTypeTag = NewMovementPaceType; }
	FORCEINLINE const FGameplayTag& GetMovementPaceType() const { return CurrentMovementPaceTypeTag; }
	
	bool ExecuteDialogueWalkRequest(const UEnvQuery* EnvQuery, float AcceptableRadius);
	bool ExecuteDialogueWalkRequest(const FVector& Location, float AcceptableRadius);
	bool ExecuteDialogueWalkRequest(const AActor* ToCharacter, float AcceptableRadius);

	void OnNpcEnteredDialogueWithPlayer(ACharacter* Character);
	void OnNpcExitedDialogueWithPlayer();
	
	FORCEINLINE AActor* GetFollowTarget() const { return FollowTarget.Get(); }
	FORCEINLINE void SetFollowTarget(AActor* NewFollowTarget) { FollowTarget = NewFollowTarget; }
	FORCEINLINE void ClearFollowTarget() { FollowTarget = nullptr; }
	
	template<typename T>
	const T* GetNpcGoalParameters(const FGameplayTag& ParameterId) const
	{
		if (const auto* ParametersInstancedStruct = NpcGoalsParameters.Find(ParameterId))
		{
			return ParametersInstancedStruct->GetPtr<T>();
		}

		return nullptr;
	}

	void StoreTaggedLocation(const FGameplayTag& DataTag, const FVector& Vector);
	void StoreTaggedActor(const FGameplayTag& DataTag, AActor* Actor);
	AActor* GetStoredActor(const FGameplayTag& DataTag, bool bConsumeAfterReading = false);
	FVector GetStoredLocation(const FGameplayTag& DataTag, bool bConsumeAfterReading = false);
	
	mutable FNpcCombatStateChanged OnStateChanged;
	mutable FNpcCombatStateChanged OnBehaviorChanged;
	mutable FNpcCombatStateChanged OnActiveAbilityChanged;

protected:
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void InitializeNpcDTR(const FNpcDTR* NpcDTR);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(RowType="/Script/ARPGAI.NpcDTR"))
	FDataTableRowHandle NpcDTRH;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TMap<FGameplayTag, TInstancedStruct<FNpcGoalParametersBase>> NpcGoalsParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGuid DesiredSquadId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bRequestSquadLeaderRole = false;
	
	bool bDead = false;
	FGameplayTag CurrentMovementPaceTypeTag = FGameplayTag::EmptyTag;
	TWeakObjectPtr<AActor> FollowTarget;
	
	TMap<FGameplayTag, FVector> StoredLocations;
	TMap<FGameplayTag, TWeakObjectPtr<AActor>> StoredActors;
	
private:
	UPROPERTY()
	TScriptInterface<INpc> OwnerNPC;
	
	UPROPERTY()
	TScriptInterface<INpcControllerInterface> NpcController;

	UPROPERTY()
	TScriptInterface<INpcAliveCreature> OwnerAliveCreature;

	TWeakObjectPtr<APawn> OwnerPawn;
	// Areas of interest of NPC: spawn zones, work areas
	
	TMap<FGameplayTag, TArray<FActiveGameplayEffectHandle>> ActiveStateEffects;
	
	UBlackboardComponent* GetBlackboardComponent() const;
	
	void EnableRagdoll(ACharacter* Mob) const;
	void RegisterDeathEvents();
	void ApplyGameplayEffectsForState_Obsolete(const FGameplayTag& StateTag, const TMap<FGameplayTag, float>& SetByCallerParams, bool bInActive, const FGameplayEffectsWrapper* StateEffects);
	void ApplyGameplayEffectsForState(const FGameplayTag& StateTag, const TMap<FGameplayTag, float>& SetByCallerParams, bool bInActive, const FGameplayEffectsWrapper* StateEffects);
	
	UFUNCTION()
	void OnNpcDeathStarted(AActor* OwningActor);

	UFUNCTION()
	void OnNpcDeathFinished(AActor* OwningActor);

	bool bNpcComponentInitialized = false; // trying to figure out how to handle world partition streaming NPC in and out

	int GroupWalkingIndex = -1;
	
	FGameplayTag CachedNpcId = FGameplayTag::EmptyTag;
};
