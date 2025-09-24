// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardAssetProvider.h"
#include "BehaviorTree/BlackboardData.h"
#include "UObject/Object.h"
#include "NpcBlackboardDataAsset.generated.h"

enum class EReactionBehaviorType : uint8;
struct FBlackboardKeySelector;
/**
 * 
 */
UCLASS()
class ARPGAI_API UNpcBlackboardDataAsset : public UDataAsset, public IBlackboardAssetProvider
{
	GENERATED_BODY()

public:
	UNpcBlackboardDataAsset();
	
	virtual UBlackboardData* GetBlackboardAsset() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UBlackboardData> Blackboard;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Common")
	FBlackboardKeySelector NpcTagsBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector StaggeredBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector AttackRangeBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector AggressivenessBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector IntelligenceBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector ReactionBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector SurroundRangeBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector CurrentTargetBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector NormalizedHealthBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector NormalizedStaminaBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector AnxietyBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector CombatEvaluationIntervalBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector DefenseActionBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	FBlackboardKeySelector IsAllEnemiesKilledBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector RequestResetGoalBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector NpcActivityStateBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector GoalExecutionTimeLeftBBKey;

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	// FBlackboardKeySelector ActiveSmartObjectClaimHandleBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector RunIndefinitelyBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector GoalTypeBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector GoalTagsBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector EqsToRunBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector EQSRunModeBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector LocationToGoBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector GestureToPlayBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector ActivityPhrasesBBKey;

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	// FBlackboardKeySelector NpcSquadFollowTypeBBKey;
	//
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	// FBlackboardKeySelector NpcSquadThreatReactionBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Follow")
	FBlackboardKeySelector FollowLeaderDotProductFactorBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Follow")
	FBlackboardKeySelector FollowLeaderDotProductDesiredDirectionBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Follow")
	FBlackboardKeySelector FollowLeaderCircleRadiusBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Follow")
	FBlackboardKeySelector FollowTargetBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector StayAtPatrolPointTimeBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector GoalStateTagBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector InteractionActorBBKey;

	// I want to replace it with conversation partner for uniformity
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	// FBlackboardKeySelector DialogueActorBBKey;

	// you could think why have interaction actor AND conversation partner as separate fields
	// but it is possible in theory that NPC is interacting with something and is in a conversation (i.e. sitting on a bench talking to either player or N)C)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Conversation")
	FBlackboardKeySelector ConversationPartnerBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Conversation")
	FBlackboardKeySelector bAcceptedConversationBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Conversation")
	FBlackboardKeySelector IsConversationPrioritizedBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Conversation")
	FBlackboardKeySelector ConversationMoveToLocationBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Conversation")
	FBlackboardKeySelector ConversationMoveToEqsBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Conversation")
	FBlackboardKeySelector ConversationSecondaryActorBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Conversation")
	FBlackboardKeySelector ConversationGoToAcceptableRadiusBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Queue")
	FBlackboardKeySelector QueuePointLocationBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Queue")
	FBlackboardKeySelector QueuePointRotationBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals|Queue")
	FBlackboardKeySelector StayAtTheBeginningOfQueueTimeBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Goals")
	FBlackboardKeySelector ExternalCompleteGoalBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	FBlackboardKeySelector ReactionIsIndefiniteBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	FBlackboardKeySelector ReactionTimeLimitBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	FBlackboardKeySelector ReactionEQSBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	FBlackboardKeySelector ReactionSpeechBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	FBlackboardKeySelector ReactionGestureBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	FBlackboardKeySelector ReactionCauserActorBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	FBlackboardKeySelector ReactionBehaviorStateBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	FBlackboardKeySelector ReactionBehaviorCustomTagsBBKey;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception Reaction Evaluator")
	TMap<EReactionBehaviorType, FBlackboardKeySelector> ReactionBehaviorUtilityKeys;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Behavior Evaluator")
	FBlackboardKeySelector ActiveBehaviorEvaluatorsTagsBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FBlackboardKeySelector> BlackboardKeysAliases;

	
};
