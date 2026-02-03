// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/NpcBlackboardDataAsset.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardData.h"
#include "EnvironmentQuery/EnvQuery.h"

UNpcBlackboardDataAsset::UNpcBlackboardDataAsset()
{
	NpcTagsBBKey.AllowNoneAsValue(true);
	StaggeredBBKey.AllowNoneAsValue(true);
	AttackRangeBBKey.AllowNoneAsValue(true);
	AggressivenessBBKey.AllowNoneAsValue(true);
	IntelligenceBBKey.AllowNoneAsValue(true);
	ReactionBBKey.AllowNoneAsValue(true);
	SurroundRangeBBKey.AllowNoneAsValue(true);
	CurrentTargetBBKey.AllowNoneAsValue(true);
	NormalizedHealthBBKey.AllowNoneAsValue(true);
	NormalizedStaminaBBKey.AllowNoneAsValue(true);
	AnxietyBBKey.AllowNoneAsValue(true);
	CombatEvaluationIntervalBBKey.AllowNoneAsValue(true);
	DefenseActionBBKey.AllowNoneAsValue(true);
	RequestResetGoalBBKey.AllowNoneAsValue(true);
	NpcActivityStateBBKey.AllowNoneAsValue(true);
	GoalExecutionTimeLeftBBKey.AllowNoneAsValue(true);
	// ActiveSmartObjectClaimHandleBBKey.AllowNoneAsValue(true);
	RunIndefinitelyBBKey.AllowNoneAsValue(true);
	GoalTypeBBKey.AllowNoneAsValue(true);
	GoalTagsBBKey.AllowNoneAsValue(true);
	EqsToRunBBKey.AllowNoneAsValue(true);
	EQSRunModeBBKey.AllowNoneAsValue(true);
	LocationToGoBBKey.AllowNoneAsValue(true);
	GestureToPlayBBKey.AllowNoneAsValue(true);
	ActivityPhrasesBBKey.AllowNoneAsValue(true);
	
	FollowLeaderDotProductFactorBBKey.AllowNoneAsValue(true);
	FollowLeaderDotProductDesiredDirectionBBKey.AllowNoneAsValue(true);
	FollowLeaderCircleRadiusBBKey.AllowNoneAsValue(true);
	FollowTargetBBKey.AllowNoneAsValue(true);
	
	InteractionActorBBKey.AllowNoneAsValue(true);
	// DialogueActorBBKey.AllowNoneAsValue(true);
	GoalStateTagBBKey.AllowNoneAsValue(true);
	QueuePointLocationBBKey.AllowNoneAsValue(true);
	QueuePointRotationBBKey.AllowNoneAsValue(true);
	StayAtTheBeginningOfQueueTimeBBKey.AllowNoneAsValue(true);
	ConversationPartnerBBKey.AllowNoneAsValue(true);
	ExternalCompleteGoalBBKey.AllowNoneAsValue(true);
	IsConversationPrioritizedBBKey.AllowNoneAsValue(true);
	
	bAcceptedConversationBBKey.AllowNoneAsValue(true);
	bAcceptedConversationBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, bAcceptedConversationBBKey));

	ConversationMoveToLocationBBKey.AllowNoneAsValue(true);
	ConversationMoveToLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, ConversationMoveToLocationBBKey));

	ConversationMoveToEqsBBKey.AllowNoneAsValue(true);
	ConversationMoveToEqsBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, ConversationMoveToEqsBBKey), UEnvQuery::StaticClass());

	ConversationGoToAcceptableRadiusBBKey.AllowNoneAsValue(true);
	ConversationGoToAcceptableRadiusBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, ConversationGoToAcceptableRadiusBBKey));

	ConversationSecondaryActorBBKey.AllowNoneAsValue(true);
	ConversationSecondaryActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, ConversationSecondaryActorBBKey), AActor::StaticClass());
	
	ConversationFollowCounterpartBBKey.AllowNoneAsValue(true);
	ConversationFollowCounterpartBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, ConversationFollowCounterpartBBKey));
	
	ActivityPhrasesBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, ActivityPhrasesBBKey)));
	NpcTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, NpcTagsBBKey)));
	GoalTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, GoalTagsBBKey)));

	IsAllEnemiesKilledBBKey.AllowNoneAsValue(true);
	IsAllEnemiesKilledBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, IsAllEnemiesKilledBBKey));

	ActiveBehaviorEvaluatorsTagsBBKey.AllowNoneAsValue(true);
	ActiveBehaviorEvaluatorsTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, ActiveBehaviorEvaluatorsTagsBBKey)));

	EQSRunModeBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UNpcBlackboardDataAsset, EQSRunModeBBKey), StaticEnum<EEnvQueryRunMode::Type>());
}

UBlackboardData* UNpcBlackboardDataAsset::GetBlackboardAsset() const
{
	return Blackboard.LoadSynchronous();
}
