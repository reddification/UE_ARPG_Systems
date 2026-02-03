#pragma once

#include "AiDataTypes.h"
#include "GameplayTagContainer.h"
#include "NpcActivitiesDataTypes.h"
#include "NpcCombatTypes.h"
#include "Engine/DataTable.h"

#include "NpcDTR.generated.h"

class UFlowAsset;
class UNpcPerceptionReactionEvaluatorsDataAsset;
class UNpcPhrasesDataAsset;
struct FNpcAttitude;
class UNpcActivityDataAsset;
class UBehaviorTree;
class UAnimMontage;
class UGameplayEffect;
class USkeletalMesh;
class UAbilitySet;

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcDTR : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Personality")
	FText Name;

	// Currently only used for more convenient setting of DTRH's RowName to an actual gameplay tag.
	// So RowName must always match the NpcId, even though it is currently not enforced programmatically 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Personality")
	FGameplayTag NpcId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Personality")
	bool IsUniqueNpc = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Behavior")
	TSoftObjectPtr<UFlowAsset> NpcActivitiesGraph;
	
	// These parameters are used by NPC goals and reaction behavior evaluators (and whatever else is added)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct), Category="Behavior")
	TMap<FGameplayTag, TInstancedStruct<FNpcGoalParametersBase>> NpcGoalAndReactionParameters;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Behavior")
	TSubclassOf<UNavigationQueryFilter> DefaultNavigationQueryFilterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Behavior")
	TArray<UNpcPerceptionReactionEvaluatorsDataAsset*> NpcPerceptionReactionEvaluators;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	TArray<UAbilitySet*> AbilitySets;

	// Faction tags, titles, etc. Can be used for attitudes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	FGameplayTagContainer DefaultTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Behavior")
	FNpcAttitudes BaseAttitudes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Behavior")
	TMap<FGameplayTag, FNpcAttitudes> CustomAttitudes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Behavior")
	UBehaviorTree* BaseBehavior;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="AI.Behavior"), Category="Behavior")
	TMap<FGameplayTag, UBehaviorTree*> DynamicBehaviors;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="AI.Behavior"), Category="Behavior")
	FGameplayTagContainer InitiallyActiveBehaviorEvaluators;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	UNpcCombatParametersDataAsset* NpcCombatParametersDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	UNpcStatesDataAsset* NpcStatesDataAsset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	class UNpcBlackboardDataAsset* NpcBlackboardDataAsset;

	// Order matters. Option which complies with requirements or empty requirements will be chosen first
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dialogue")
	TArray<UNpcPhrasesDataAsset*> NpcPhrasesDataAssets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	TSoftClassPtr<ACharacter> SpawnClass;
};