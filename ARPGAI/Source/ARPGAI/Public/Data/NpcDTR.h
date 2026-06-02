#pragma once

#include "AiDataTypes.h"
#include "GameplayTagContainer.h"
#include "NpcActivitiesDataTypes.h"
#include "Engine/DataTable.h"

#include "NpcDTR.generated.h"

class UNpcBehaviorsConfiguration;
class UNpcCombatParametersDataAsset;
class UFlowAsset;
class UNpcPerceptionReactionEvaluatorsDataAsset;
class UPhrasesDataAsset;
struct FAttitude;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Behavior")
	TSoftObjectPtr<UFlowAsset> NpcActivitiesGraph;
	
	// These parameters are used by NPC goals and reaction behavior evaluators (and whatever else is added)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct), Category="AI|Behavior")
	TMap<FGameplayTag, TInstancedStruct<FNpcGoalParametersBase>> NpcGoalAndReactionParameters;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Navigation")
	TSubclassOf<UNavigationQueryFilter> DefaultNavigationQueryFilterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Behavior")
	TArray<UNpcPerceptionReactionEvaluatorsDataAsset*> NpcPerceptionReactionEvaluators;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Behavior")
	TSoftObjectPtr<UNpcBehaviorsConfiguration> BehaviorsConfiguration;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attitudes")
	FAttitudeSet BaseAttitudes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attitudes")
	TMap<FGameplayTag, FAttitudeSet> CustomAttitudes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	TArray<UAbilitySet*> AbilitySets;

	// Faction tags, titles, etc. Can be used for attitudes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	FGameplayTagContainer DefaultTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	UNpcCombatParametersDataAsset* NpcCombatParametersDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	UNpcStatesDataAsset* NpcStatesDataAsset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	class UNpcBlackboardDataAsset* NpcBlackboardDataAsset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	TSoftClassPtr<ACharacter> SpawnClass;
	
	// Item tags or categories that are valueable to this NPC
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configuration")
	FGameplayTagContainer ValueableItems;
};