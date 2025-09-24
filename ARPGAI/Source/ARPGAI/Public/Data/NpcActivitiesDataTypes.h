#pragma once

#include "GameplayTagContainer.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "NavFilters/NavigationQueryFilter.h"

#include "NpcActivitiesDataTypes.generated.h"

class UNpcPerceptionReactionEvaluatorsDataAsset;

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
	Hunt,
};

enum ENpcGoalAdvanceResult
{
	Completed,
	Failed,
	InProgress
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery CharacterFilter;

	// In what range to conversation initiator should characters be searched for
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SearchInRange = 500.f;

	// How many characters with this tag id to include if there's more than one
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Count = 1;
};

enum EActivityOriginType
{
	None,
	Lifecycle,
	Quest
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

USTRUCT(BlueprintType)
struct FNpcSquadMemberFollowParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ENpcSquadFollowType NpcSquadFollowType = ENpcSquadFollowType::Around;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DotProductScore = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FollowRadius = 250.f;
};

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcGoalParametersBase
{
	GENERATED_BODY()
	
};

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcGoalParameters_UseSmartObject : public FNpcGoalParametersBase
{
	GENERATED_BODY()

	// Location in which radius to search for smart objects. If not set - player will be used
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag LocationIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LocationSearchRadius = 5000.f;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery IntentionFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery SmartObjectActorFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EEnvQueryRunMode::Type> EqsRunMode = EEnvQueryRunMode::Type::RandomBest5Pct;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bRepeatUntilNoInteractableActorsLeft = false;	
	
	// TODO implement usage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	int RequiredInteractionsCount = 1;	
};

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcGoalParameters_FollowPath : public FNpcGoalParametersBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag RouteId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bStopAtPathPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bStopAtPathPoint))
	FGameplayTag GestureAtPatrolPoint;

	// If false and there are multiple routes with the same tag - random route will be picked.
	// Otherwise route with the closest point will be picked
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bStopAtPathPoint))
	bool bPreferClosestRoute = true;	

	// Use pathfinding to find closest patrol route. Might make the difference if there are some high city walls or whatever
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bPreferClosestRoute"))
	bool bUsePathfinding = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0, ClampMin = 0))
	int Loops = 0;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bStopAtPathPoint))
	float StayAtEachPatrolPointTimeMin = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bStopAtPathPoint))
	float StayAtEachPatrolPointTimeMax = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCompleteOnReachingEdge = false;
};

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcGoalParameters_LocationLimitation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categoried="G2VS2.Location"))
	FGameplayTag LocationId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bMustBeInside = true;
};

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcGoalParameters_StayInQueue : public FNpcGoalParametersBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag QueueId;

	// What gesture NPC will play while in queue. Usually something idle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag OptionalGestureTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStayInQueueIndefinitely = false;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bStayInQueueIndefinitely == false"))
	float FirstInQueueGameTimeDuration = 0.f;

	// -1 means there's no desired position. if there are multiple NPCs that want the same position - only 1st will occupy it.
	// Other will only next available place in queue  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(UIMin = -1, ClampMin = -1))
	int DesiredQueuePosition = -1;
};

USTRUCT(BlueprintType)
struct FNpcGoalParameters_Conversate : public FNpcGoalParametersBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ConversationId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseEQS = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIncludePlayer = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "bUseEQS == false"))
	FGameplayTag ConversationPartnerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "bUseEQS == false"))
	TArray<FNpcConversationSecondaryParticipantData> SecondaryConversationParticipants;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "bUseEQS"))
	UEnvQuery* ConversationPartnersEQS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ConversationPartnerTagsFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bForceConversationPartnerSuspendActivity = false;
};

USTRUCT(BlueprintType)
struct FNpcGoalParameters_TalkToPlayer : public FNpcGoalParametersBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag OptionalDialogueId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bGoToPlayerDirectly = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="!bGoToPlayerDirectly"))
	UEnvQuery* PlayerSearchEQS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FNpcConversationSecondaryParticipantData> SecondaryConversationParticipants;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bInterruptActivePlayerInteraction = false;
};