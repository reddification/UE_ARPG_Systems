#pragma once
#include "BlackboardKeyType_GameplayTag.h"
#include "GameplayTagContainer.h"
#include "Components/NpcQueueComponent.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "Data/NpcDTR.h"
#include "Engine/DataTable.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

#include "NpcGoals.generated.h"

class UNpcGoalBackgroundTaskBase;
class UNpcQueueComponent;
struct FNpcGoalChain;
class UBlackboardComponent;
class UEnvQuery;
class UNpcBlackboardDataAsset;

UENUM()
enum class ENpcGoalType : uint8
{
	None = 0,
	VisitLocation,
	FindArea,
	FollowLeader,
	UseSmartObject,
	Gesture,
	Wander,
	Conversate,
	TalkToPlayer,
	Patrol,
	StayInQueue,
};

enum ENpcGoalAdvanceResult
{
	Completed,
	Failed,
	InProgress
};

// This basically should set the score for following EQS dot product test
UENUM()
enum class ENpcSquadFollowType : uint8 
{
	Around, // dot-product independent, squad coordinator component dependent
	InFront, // prefer dot product -> 1
	NextTo, // prefer dot product -> 0
	Behind, // prefer dot product -> -1
	NotInFrontOf // prefer dot product farther from 1
};

UENUM()
enum class ENpcSquadThreatReaction : uint8 
{
	None,
	// Find threats on its own
	ProActive,
	// Only receive threats from leader
	Passive,
	// don't engage in combat with threats at all until personally touched
	Neutral,
	// Stay away/run away from threats
	Cautious,
};

UENUM()
enum class ENpcGoalStartResult : uint8 
{
	None,
	InProgress,
	Finished,
	Failed
};

USTRUCT(BlueprintType)
struct FNpcConversationSecondaryParticipantData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CharacterId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery CharacterFilter;

	// In what range to conversation initiator should characters be searched for
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SearchInRange = 500.f;

	// How many characters with this tag id to include if there's more than one
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int Count = 1;
};

// in theory, goals can be shared (flat hierarchy NPCs), assigned (leader-follower hierarchy) and individual (not related to other NPCs)

UCLASS(Abstract, EditInlineNew)
class ARPGAI_API UNpcGoalBase : public UObject
{
	GENERATED_BODY()

	friend class UNpcActivityComponent;
	
public:
	UNpcGoalBase();
	const FGuid& GetGoalId() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FNpcGoalChain SubordinateGoalChain;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Character.State,G2VS2.Character.State"))
	FGameplayTag CharacterStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer GrantedTagsDuringGoal;

	// Do whatever you want with these. In G2VS2 these are used to notify quest system that some quest behavior goal has been completed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer CustomGoalTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttitudePreset;

	virtual FString GetDescription() const;

	virtual ENpcGoalAdvanceResult AdvanceGoal(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
	                                          UNpcActivityComponent* NpcActivityComponent, bool bCurrentPhaseResult, const FGameplayTagContainer&
	                                          GoalExecutionResultTags);

	// Suspend should be called when NPC stopped executing goal for a short time, like when a player initiated dialogue with it or it had to react to a threat
	// Like some external factor is distracting NPC but when this source of distraction is gone - NPC is returning to the goal
	// So if EndGoal is intended to completely finalize the goal and release resources,
	// SuspendGoal should most of the time "resetting" player state (i.e. removing granted tags, but keeping place in queue occupied, etc)
	virtual void SuspendGoal(UNpcActivityComponent* NpcActivityComponent);
	virtual void EndGoal(UNpcActivityComponent* NpcActivityComponent);

	virtual ENpcGoalStartResult Restore(UBlackboardComponent* BlackboardComponent,
	                                    const UNpcBlackboardDataAsset* BlackboardKeys,
	                                    UNpcActivityComponent* NpcActivityComponent, bool bInitialStart) const;

protected:
	ENpcGoalType NpcGoalType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid GoalId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<UNpcGoalBackgroundTaskBase*> BackgroundTasks;
	
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent,
	                                  const UNpcBlackboardDataAsset* BlackboardKeys,
	                                  UNpcActivityComponent* NpcActivityComponent) const;
};

UCLASS()
class ARPGAI_API UNpcGoalUseSmartObject : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalUseSmartObject() { NpcGoalType = ENpcGoalType::UseSmartObject; }
	virtual ENpcGoalAdvanceResult AdvanceGoal(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent* NpcActivityComponent,
		bool bCurrentPhaseResult, const FGameplayTagContainer& GoalExecutionResultTags) override;

	virtual FString GetDescription() const override;

	FNpcGoalParameters_UseSmartObject GetParameters(const APawn* GoalExecutor) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EEnvQueryRunMode::Type> EqsRunMode = EEnvQueryRunMode::Type::RandomBest5Pct;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag NpcGoalDataParameterId;
	
	// Location in which radius to search for smart objects. If not set - player will be used
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="NpcGoalDataParameterId.IsValid() == false"))
	FGameplayTag LocationIdTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="NpcGoalDataParameterId.IsValid() == false"))
	float LocationSearchRadius = 5000.f;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="NpcGoalDataParameterId.IsValid() == false"))
	FGameplayTagQuery IntentionFilter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="NpcGoalDataParameterId.IsValid() == false"))
	FGameplayTagQuery SmartObjectActorFilter;

	// TODO implement usage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRepeatUntilNoInteractableActorsLeft = false;	
	
	// TODO implement usage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	int RequiredInteractionsCount = 1;	
	
	
protected:
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent,
	                                  const UNpcBlackboardDataAsset* BlackboardKeys,
	                                  UNpcActivityComponent* NpcActivityComponent) const override;
};

UCLASS()
class ARPGAI_API UNpcGoalVisitLocation : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalVisitLocation() { NpcGoalType = ENpcGoalType::VisitLocation; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag LocationIdTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bCompleteOnEntering = false;	
	
protected:
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent,
	                                  const UNpcBlackboardDataAsset* BlackboardKeys,
	                                  UNpcActivityComponent* NpcActivityComponent) const override;
};

UCLASS()
class ARPGAI_API UNpcGoalGesture : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalGesture() { NpcGoalType = ENpcGoalType::Gesture; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag GestureTag;

protected:
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent,
	                                  const UNpcBlackboardDataAsset* BlackboardKeys,
	                                  UNpcActivityComponent* NpcActivityComponent) const override;
};

UCLASS()
class ARPGAI_API UNpcGoalWander : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalWander() { NpcGoalType = ENpcGoalType::Wander; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UEnvQuery* AreaEqs;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer GestureOptionsTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer SpeechOptionsTags;
	
protected:
	virtual ENpcGoalStartResult Restore(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
		UNpcActivityComponent* NpcActivityComponent, bool bInitialStart) const override;
};

UCLASS()
class ARPGAI_API UNpcGoalFollowLeader : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalFollowLeader() { NpcGoalType = ENpcGoalType::FollowLeader; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ENpcSquadFollowType NpcSquadFollowType = ENpcSquadFollowType::Around;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DotProductScore = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FollowRadius = 250.f;
	
protected:
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent,
	                                  const UNpcBlackboardDataAsset* BlackboardKeys,
	                                  UNpcActivityComponent* NpcActivityComponent) const override;
};

UCLASS()
class ARPGAI_API UNpcGoalFindArea : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalFindArea() { NpcGoalType = ENpcGoalType::FindArea; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UEnvQuery* AreaEqs;

protected:
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent,
	                                  const UNpcBlackboardDataAsset* BlackboardKeys,
	                                  UNpcActivityComponent* NpcActivityComponent) const override;
};

UCLASS()
class ARPGAI_API UNpcGoalConversate : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalConversate() { NpcGoalType = ENpcGoalType::Conversate; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ConversationId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bUseEQS = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIncludePlayer = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition = "bUseEQS == false"))
	FGameplayTag ConversationPartnerId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition = "bUseEQS == false"))
	TArray<FNpcConversationSecondaryParticipantData> SecondaryConversationParticipants;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition = "bUseEQS"))
	UEnvQuery* ConversationPartnersEQS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery ConversationPartnerTagsFilter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bForceConversationPartnerSuspendActivity = false;

protected:
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent,
	                                  const UNpcBlackboardDataAsset* BlackboardKeys, UNpcActivityComponent*
	                                  NpcActivityComponent) const override;
	virtual ENpcGoalStartResult Restore(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
		UNpcActivityComponent* NpcActivityComponent, bool bInitialStart) const override;

	bool SetConversationBlackboardContext(UBlackboardComponent* BlackboardComponent,
										  const UNpcBlackboardDataAsset* BlackboardKeys,
										  UNpcActivityComponent* NpcActivityComponent) const;
};

UCLASS()
class ARPGAI_API UNpcGoalTalkToPlayer : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalTalkToPlayer() { NpcGoalType = ENpcGoalType::TalkToPlayer; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag OptionalDialogueId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bGoToPlayerDirectly = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="!bGoToPlayerDirectly"))
	UEnvQuery* PlayerSearchEQS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FNpcConversationSecondaryParticipantData> SecondaryConversationParticipants;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bInterruptActivePlayerInteraction = false;

protected:
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
		UNpcActivityComponent* NpcActivityComponent) const override;
};

UCLASS()
class ARPGAI_API UNpcGoalPatrol : public UNpcGoalBase
{
	GENERATED_BODY()
	
public:
	UNpcGoalPatrol() { NpcGoalType = ENpcGoalType::Patrol; }

	FNpcGoalParameters_Patrol GetParameters(const APawn* GoalExecutor) const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag NpcGoalDataParameterId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="NpcGoalDataParameterId.IsValid() == false"))
	FGameplayTag PatrolRouteId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag GestureAtPatrolPoint;

	// If false and there are multiple routes with the same tag - random route will be picked.
	// Otherwise route with the closest point will be picked
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bPreferClosestRoute = true;	

	// Use pathfinding to find closest patrol route. Might make the difference if there are some high city walls or whatever
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bPreferClosestRoute"))
	bool bUsePathfinding = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0, ClampMin = 0))
	int Loops = 0;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StayAtEachPatrolPointTimeMin = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StayAtEachPatrolPointTimeMax = 20.f;
	
	virtual ENpcGoalAdvanceResult AdvanceGoal(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
	                                          UNpcActivityComponent* NpcActivityComponent, bool bCurrentPhaseResult, const FGameplayTagContainer&
	                                          GoalExecutionResultTags) override;
	virtual void EndGoal(UNpcActivityComponent* NpcActivityComponent) override;
	
protected:
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
	                                  UNpcActivityComponent* NpcActivityComponent) const override;
	
	virtual ENpcGoalStartResult Restore(UBlackboardComponent* BlackboardComponent,
	                                    const UNpcBlackboardDataAsset* BlackboardKeys,
	                                    UNpcActivityComponent* NpcActivityComponent, bool bInitialStart) const override;

private:
	void UpdateStayAtPatrolPointTime(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
		UNpcActivityComponent* NpcActivityComponent) const;
};


UCLASS()
class UNpcGoalStayInQueue : public UNpcGoalBase
{
	GENERATED_BODY()

public:
	UNpcGoalStayInQueue() { NpcGoalType = ENpcGoalType::StayInQueue; }
	
	virtual ENpcGoalStartResult Start(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
		UNpcActivityComponent* NpcActivityComponent) const override;
	
	virtual ENpcGoalAdvanceResult AdvanceGoal(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
		UNpcActivityComponent* NpcActivityComponent, bool bCurrentPhaseResult, const FGameplayTagContainer& GoalExecutionResultTags) override;
	
	virtual ENpcGoalStartResult Restore(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* BlackboardKeys,
	                                    UNpcActivityComponent* NpcActivityComponent, bool bInitialStart) const override;

	virtual void SuspendGoal(UNpcActivityComponent* NpcActivityComponent) override;
	void UpdateQueuePosition(UBlackboardComponent* BlackboardComponent, const UNpcBlackboardDataAsset* NpcBlackboardDataAsset,
		const FNpcQueueMemberPosition& NpcQueueMemberPosition) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag QueueId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag QueueGestureTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bStayInQueueIndefinitely = false;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bStayInQueueIndefinitely == false"))
	float FirstInQueueGameTimeDuration = 0.f;

	// Use these to append a random tag from a container to the NPC when it is staying in line
	// Could be used to apply different gaits/postures/stances
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer OptionalTagWhenInQueue;

private:
	struct FNpcGoalMemory_StayInQueue
	{
		TWeakObjectPtr<UNpcQueueComponent> NpcQueue;
		FGameplayTag AppliedOptionalTag;
	};
};
