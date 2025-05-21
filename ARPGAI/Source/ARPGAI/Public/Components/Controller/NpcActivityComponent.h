#pragma once

#include "CoreMinimal.h"
#include "SmartObjectRuntime.h"
#include "Activities/NpcGoals.h"
#include "Data/NpcSettings.h"
#include "Components/ActorComponent.h"
#include "Components/NpcQueueComponent.h"
#include "Data/AiDataTypes.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "NpcActivityComponent.generated.h"

class INpc;
struct FNpcGoalChain;
class UBlackboardComponent;
class UNpcComponent;
struct FNpcDTR;
class INpcControllerInterface;
class UBehaviorTreeComponent;
class UNpcActivityDataAsset;
class UNpcGoalBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcActivityComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UNpcActivitySquadSubsystem;
	DECLARE_MULTICAST_DELEGATE_TwoParams(FNpcGoalCompletedEvent, const FGuid& GoalId, const FGameplayTagContainer& Tags)
	
private:
	enum EActivityOriginType
	{
		None,
		Lifecycle,
		Subordinate,
		Quest
	};
	
public:
	UFUNCTION(BlueprintCallable)
	void RunCustomBehavior(const FGameplayTag& CustomBehaviorTag);
	
	UFUNCTION(BlueprintCallable)
	void StopCustomBehavior();

	void OnNpcInteractionStateChanged(AActor* InteractionInstigator, const FGameplayTag& InteractionTypeTag, bool bActive);
	void OnNpcReachedLocation(const FGameplayTagContainer& WorldLocationTags);

	void InitializeNpc(ACharacter* Npc, const FNpcDTR* NpcDtr);
	void SetNavigationFilterClass(const TSubclassOf<UNavigationQueryFilter>& CustomNavigationFilter);
	void ResetNavigationFilterClass();
	void SetDayTime(const FGameplayTag& DayTimeTag);

	void EnterDialogue(AActor* InteractionInstigator);
	void ExitDialogue();

	FVector GetPawnLocation() const;

public:
	const FGameplayTag& GetActivityStateTag() const;

	virtual void StartNewActivity(const UNpcActivityDataAsset* InActivityParameters);

	const UNpcActivityDataAsset* GetActivitySettings() const { return CurrentActivity; }

	bool RequestNextNpcGoal();
	void SuspendActiveGoal();

	void SetSubordinateNpcGoal(const FNpcGoalChain& SubordinateGoalChain);
	void ResetSubordinateGoal();
	bool SetActivityGoalData();
	void SetRemainingGoalExecutuionTime(const float InTimeLeft);

	// bool TryClaimSmartObject(FSmartObjectClaimHandle& SmartObjectClaimHandle, AActor* SmartObject, bool bTryReuseLastSlot = true);
	// bool TryReclaimSmartObject();
	
	void LeaveSquad();
	void SetActivityLocation(const FGameplayTag& ActivityLocationIdTag);
	const UNpcGoalBase* GetActiveGoal() const;
	const FGuid& GetSquadId() const { return SquadId; }
	APawn* GetNpcPawn() const;
	bool IsAtLocation(const FGameplayTag& LocationId) const;
	const UNpcActivityDataAsset* GetRunningActivity() const { return CurrentActivity; }
	const FNpcGoalChain& GetActiveGoalChain() const { return ActiveNpcGoalChain; }
	ENpcGoalAdvanceResult AdvanceCurrentGoal(bool bCurrentGoalStepResult, const FGameplayTagContainer& GoalExecutionResultTags);

	void OnNpcQueueMemberAdvanced(AActor* Npc, const FNpcQueueMemberPosition& NpcQueueMemberPosition);

	uint8* AllocateGoalMemory(const FGuid& GoalId, size_t Size);
	uint8* GetGoalMemory(const FGuid& GoalId);
	void ClearGoalMemory(const FGuid& GoalId);
	void ExternalCompleteGoal();

	mutable FNpcGoalCompletedEvent NpcGoalCompletedEvent;

protected:
	UPROPERTY()
	const UNpcActivityDataAsset* CurrentActivity;

	int32 NpcGoalIndex = -1;
	int32 ActiveNpcGoalChainIndex = -1;
	
	UPROPERTY()
	FNpcGoalChain ActiveNpcGoalChain;

	UPROPERTY()
	FNpcGoalItem ActiveNpcGoal;
	
	EActivityOriginType CurrentActivityOriginType = EActivityOriginType::None;
	
	float RemainingGoalExecutionTime = 0.f;
	bool bGoalAssessed = false;
	FGameplayTag CurrentActivityLocationIdTag;
	bool bSquadLeader = false;
	FGameplayTagContainer InteractionsInterruptedByDialogue;
	FGameplayTagQuery InterruptCurrentActivityForDialogueWhenNpcInStateFilter;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	const UNpcSettings* GetNpcSettings() const;
	void OnNpcDied(AActor* DeadNpc);
	void RequestRestartBehaviorTreeGoalExecution();
	void RunActivity(const FNpcDTR* NpcDTR, const FGameplayTag& ActivityTag);
	void StartActivity(const FNpcDTR* NpcDTR, UNpcActivityDataAsset* NewActivity);
	void StartDelayedDailyActivity();

	TWeakObjectPtr<UNpcComponent> NpcComponent;
	FTimerHandle DailyActivityDelayTimer;
	float GameTimeToRealTimeCoefficient = 1.f;
	float MinDailyActivityDelayGameTime = 0.25f;
	float MaxDailyActivityDelayGameTime = 1.f;
	FGameplayTag DayTimeTag;
	
	UPROPERTY()
	TScriptInterface<INpcControllerInterface> NpcController;

	UPROPERTY()
	TScriptInterface<INpc> Npc;
	
	TWeakObjectPtr<class UBlackboardComponent> BlackboardComponent;
	TSubclassOf<UNavigationQueryFilter> InitialNavigationFilterClass;

	TMap<FGuid, uint8*> GoalsMemories;
	
	FGuid SquadId;
};
