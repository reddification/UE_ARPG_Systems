#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/QuestActions.h"
#include "Engine/DataTable.h"
#include "Interfaces/QuestNPC.h"
#include "Containers/Map.h"
#include "Data/QuestActionStopConditions.h"
#include "QuestNpcSubsystem.generated.h"

struct FGameplayTag;
class IQuestNPC;

UCLASS()
class QUESTSYSTEM_API UQuestNpcSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	struct FCachedNpcAttitude
	{
		FGameplayTagContainer TargetCharacters;
		FGameplayTag Attitude;
		FDateTime ValidUntilGameTime;
	};
	
public:
	static UQuestNpcSubsystem* Get(const UObject* WorldContextObject);
	
	void RegisterNpc(const FGameplayTag& NpcIdTag, AActor* Npc);
	void UnregisterNpc(const FGameplayTag& NpcIdTag, AActor* Npc);
	
	bool TryRunQuestBehavior(const FQuestActionNpcRunBehavior& RunBehaviorData, const FGuid& QuestActionId, const FQuestSystemContext&
	                         QuestSystemContext);
	bool RunQuestBehavior(const TScriptInterface<IQuestNPC>& Npc, const FNpcQuestBehaviorDescriptor& BehaviorDescriptor, const FGuid&
	                      QuestActionId, const FQuestSystemContext& QuestSystemContext);
	void OnQuestBehaviorConditionTriggered(TWeakInterfacePtr<IQuestNPC> Npc, const FGuid& BehaviorActionId);
	
	TScriptInterface<IQuestNPC> FindAndTeleportExistingNpc(const FGameplayTag& NpcIdTag, const FVector& SpawnLocation, const FVector& PlayerLocation);
	
	void ChangeTagsForNpcs(const FGameplayTag& NpcId, const FGameplayTagContainer& TagsToUpdate, bool bAdd, const FGameplayTagQuery* NpcFilter = nullptr);
	TScriptInterface<IQuestNPC> FindNpc(const FGameplayTag& NpcIdTag, const FVector& PlayerLocation);
	void AddCustomAttitude(const FGameplayTagContainer& SourceCharacterIds, const FGameplayTagContainer& TargetCharacterIds, const FGameplayTag& NewAttitude, const
	                       float GameTimeLimit);
	void SetCustomAttitudePreset(const FGameplayTagContainer& ForCharacterIds, const FGameplayTag& NewAttitude, float GameHoursDuration);

	const FGameplayTag& GetForcedAttitude(const FGameplayTag& NpcIdTag, const FGameplayTagContainer& ActorTags, const FDateTime& GameTime);

	TArray<TScriptInterface<IQuestNPC>> GetNpcs(const FGameplayTag& NpcId, const FGameplayTagQuery* OptionalFilter = nullptr) const;
	TArray<TScriptInterface<IQuestNPC>> GetNpcsInRange(const FGameplayTag& NpcId, const FVector& QuerierLocation, float Range, const FGameplayTagQuery* OptionalFilter = nullptr) const;
	
private:
	//UPROPERTY()
	TMultiMap<FGameplayTag, TScriptInterface<IQuestNPC>> Npcs;

	UPROPERTY()
	TMap<FGuid, FNpcQuestBehaviorEndConditionContainer> QuestBehaviorEndConditions;

	TMap<FGameplayTag, TArray<FCachedNpcAttitude>> CustomAttitudes;
};
