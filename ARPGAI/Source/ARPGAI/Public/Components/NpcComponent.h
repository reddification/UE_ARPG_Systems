#pragma once

#include "CoreMinimal.h"
#include "NpcSpawnerComponent.h"
#include "BehaviorTree/Tasks/BTTask_Attack.h"
#include "Controller/NpcActivityComponent.h"
#include "Data/AiDataTypes.h"
#include "Data/NpcDTR.h"
#include "GAS/Attributes/NpcCombatAttributeSet.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "NpcComponent.generated.h"

struct FNpcAttitudes;
class INpcAliveCreature;
class INpc;
class INpcControllerInterface;

class UBlackboardComponent;
struct FBlackboardKeySelector;

struct FTemporaryCharacterAttitudeMemory
{
	FGameplayTag AttitudeTag;
	FTimespan ValidUntilGameTime = 0.f;
};

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
	
	FORCEINLINE const UNpcSpawnerComponent* GetDesignatedZone() const { return DesignatedZone.Get(); }
	FORCEINLINE void SetDesignatedZone(const UNpcSpawnerComponent* InNpcSpawnerZone) { DesignatedZone = InNpcSpawnerZone; }
	
	const FNpcDTR* GetNpcDTR() const;

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetAttitude(const AActor* Actor) const;
	
	void OnDamageReceived(float DamageAmount, const FOnAttributeChangeData& ChangeData);

	const FGameplayTag& GetNpcIdTag() const;

	void AddTemporaryCharacterAttitude(const AActor* Character, const FGameplayTag& Attitude);

	void SetAttitudePreset(const FGameplayTag& InAttitudePreset);
	void SetTemporaryAttitudePreset(const FGameplayTag& InAttitudePreset);
	void ResetTemporaryAttitudePreset();

	void SetStateActive(const FGameplayTag& StateTag, const TMap<FGameplayTag, float>& SetByCallerParams, bool bInActive);
	
	FGameplayTagContainer GetNpcTags() const;
	const struct FNpcRealtimeDialogueLine* GetDialogueLine(const FGameplayTag& LineTagId) const;
	const FGameplayTag& GetCurrentAttitudePreset() const;

	FORCEINLINE void SetMovementPaceType(const FGameplayTag& NewMovementPaceType) { CurrentMovementPaceTypeTag = NewMovementPaceType; }
	FORCEINLINE const FGameplayTag& GetMovementPaceType() const { return CurrentMovementPaceTypeTag; }
	
	bool ExecuteDialogueWalkRequest(const UEnvQuery* EnvQuery, float AcceptableRadius);
	bool ExecuteDialogueWalkRequest(const FVector& Location, float AcceptableRadius);
	bool ExecuteDialogueWalkRequest(const AActor* ToCharacter, float AcceptableRadius);

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
	
	void SetHostile(AActor* ToActor);

	mutable FNpcCombatStateChanged OnStateChanged;
	mutable FNpcCombatStateChanged OnBehaviorChanged;
	mutable FNpcCombatStateChanged OnActiveAbilityChanged;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void InitializeNpcDTR(const FNpcDTR* NpcDTR);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(RowType="/Script/ARPGAI.NpcDTR"))
	FDataTableRowHandle NpcDTRH;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TMap<FGameplayTag, TInstancedStruct<FNpcGoalParametersBase>> NpcGoalsParameters;
	
private:
	void SetAttitudePresetInternal(const FGameplayTag& InAttitudePreset);
	
	UPROPERTY()
	TScriptInterface<INpc> OwnerNPC;
	
	UPROPERTY()
	TScriptInterface<INpcControllerInterface> NpcController;

	UPROPERTY()
	TScriptInterface<INpcAliveCreature> OwnerAliveCreature;

	TWeakObjectPtr<APawn> OwnerPawn;
	TWeakObjectPtr<const UNpcSpawnerComponent> DesignatedZone;
	
	TMap<FGameplayTag, TArray<FActiveGameplayEffectHandle>> ActiveStateEffects;
	
	UBlackboardComponent* GetBlackboardComponent() const;
	
	void EnableRagdoll(ACharacter* Mob) const;
	void RegisterDeathEvents();

	UFUNCTION()
	void OnNpcDeathStarted(AActor* OwningActor);

	UFUNCTION()
	void OnNpcDeathFinished(AActor* OwningActor);

	bool bDead = false;
	bool bNpcComponentInitialized = false; // trying to figure out how to handle world partition streaming NPC in and out

	FGameplayTag CurrentAttitudePreset = FGameplayTag::EmptyTag;
	FGameplayTag CurrentTemporaryAttitudePreset = FGameplayTag::EmptyTag;
	FGameplayTag CurrentMovementPaceTypeTag = FGameplayTag::EmptyTag;
	TMap<FGameplayTag, float> NpcAttitudesDurationGameTime;
	FNpcAttitudes BaseAttitudes;
	FNpcAttitudes CustomAttitudes;
	mutable TMap<TWeakObjectPtr<const AActor>, FTemporaryCharacterAttitudeMemory> TemporaryCharacterAttitudes;
	TWeakObjectPtr<AActor> FollowTarget;
	
	TMap<FGameplayTag, FVector> StoredLocations;
	TMap<FGameplayTag, TWeakObjectPtr<AActor>> StoredActors;
};
