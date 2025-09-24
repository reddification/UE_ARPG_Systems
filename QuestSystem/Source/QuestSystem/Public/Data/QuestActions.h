#pragma once

#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "QuestRequirements.h"
#include "QuestTypes.h"
#include "Interfaces/DelayedQuestActionInterface.h"
#include "QuestActions.generated.h"

struct FNpcQuestBehaviorEndConditionBase;
class UArbitraryQuestAction;
// struct FQuestRequirementBase;
struct FQuestSystemContext;

USTRUCT(BlueprintType)
struct FQuestSpawnBaseAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bNearPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "bNearPlayer == true"))
	float NearPlayerRadius = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Location.Id,Location.Id", EditCondition = "bNearPlayer == false"))
	FGameplayTag LocationIdTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer WithTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FloorOffset = 90.f;
};

USTRUCT(BlueprintType)
struct FNpcQuestBehaviorDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Npc.Activity,Npc.Activity"))
	FGameplayTag RequestedBehaviorIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FNpcQuestBehaviorEndConditionBase>> QuestBehaviorEndConditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAnyEndConditionIsEnough = true;	
};

USTRUCT(BlueprintType)
struct FQuestActionNpcRunBehavior
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer NpcIdsTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bFirstOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bPickNpcClosestToPlayer = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcQuestBehaviorDescriptor NpcQuestBehaviorDescriptor;
};

USTRUCT(BlueprintType)
struct FQuestActionSpawnNpcAndSetBehavior : public FQuestSpawnBaseAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag NpcIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSpawnNew = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcQuestBehaviorDescriptor OptionalNpcInitialBehavior;
};

USTRUCT(BlueprintType)
struct FQuestActionSpawnItems : public FQuestSpawnBaseAction
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Item,Item"))
	FGameplayTag ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Item,Item"))
	FGameplayTag ItemCategory;
};

class UQuestActionProxy;

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FQuestActionBase
{
	GENERATED_BODY()

public:
	FQuestActionBase();
	virtual ~FQuestActionBase() = default;

	FORCEINLINE bool IsDelayed() const { return GameTimeDelayHours > 0.f || StartAtNextTimeOfDay.IsValid(); }
	FORCEINLINE bool IsEnabled() const { return bEnabled; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString UserDescription;

	// Must be persistent (hence not re-generated in between game starts)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGuid ActionId;

	// Disabled actions are not executed. Useful for debugging purposes
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnabled = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FQuestRequirementBase>> ExecuteActionQuestRequirements;

	// If both set, StartAtNextTimeOfDay has a priority over GameTimeDelayHours
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ActionId.IsValid()"))
	FGameplayTag StartAtNextTimeOfDay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ActionId.IsValid()"))
	float GameTimeDelayHours = 0.f;
	
	void Execute(const FQuestSystemContext& Context) const;
	bool CanExecute(const FQuestSystemContext& Context) const;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const;
};

UCLASS()
class UQuestActionProxy : public UObject, public IDelayedQuestAction
{
	GENERATED_BODY()
	
public:
	void Initialize(const TInstancedStruct<FQuestActionBase>& Action, const FQuestSystemContext& Context);	
	virtual void StartDelayedAction(const FQuestSystemContext& QuestSystemContext) override;
	const FGuid& GetActionId() const { return ActionId; };
	
private:
	TInstancedStruct<FQuestActionBase> QuestAction;
	FQuestSystemContext CachedQuestSystemContext;
	FGuid ActionId;
};

USTRUCT()
struct FQuestActionRunNpcBehavior : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FQuestActionNpcRunBehavior QuestActionNpcRunBehavior;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionSpawnNpc : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FQuestActionSpawnNpcAndSetBehavior SpawnNpcAndSetBehaviorData;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionSpawnItem : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FQuestActionSpawnItems SpawnItemData;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FItemChangeData
{
	GENERATED_BODY()

	// Can be negative. In this case amount will be subtracted from inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Count = 1;
};

USTRUCT()
struct FQuestActionUpdateCharacterInventory : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FItemChangeData> ItemsChange;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionDestroyActors : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery GameplayTagFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float OverTime = 0.f;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionUpdateWorldState : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer WorldStateTagsChange;
	
	// if false - tags will be removed from world state
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAppend = true;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionChangeTagsOnPlayer : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Tags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAdd = true;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionChangeTagsOnNpcs : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Tags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery CharacterFilter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAdd = true;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionSetPlayerState : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> SetByCallerParams;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAdd = true;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionSetNpcState : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery CharacterFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> SetByCallerParams;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAdd = true;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionStartQuests : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(RowType=QuestDTR))
	TArray<FDataTableRowHandle> QuestsToStart;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionPlayCutscene : public FQuestActionBase
{
	GENERATED_BODY()

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionGiveXP : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0, ClampMin = 0))
	int XPReward = 0;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionGrantAbilitySet : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	mutable TSoftObjectPtr<class UAbilitySet> GrantedAbilitySet;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionSetAttitude : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer SourceCharacterIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer TargetCharacterIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.Attitude,G2VS2.Npc.Attitude"))
	FGameplayTag NewAttitude;

	// If less than or equal to zero - consider infinite 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin = -1.f, UIMin = -1.f))
	float GameHoursDuration = -1.f;
	
protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionSetAttitudePreset : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer ForCharacterIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.Attitude,G2VS2.Npc.Attitude"))
	FGameplayTag NewAttitudePreset;

	// If less than or equal to zero - consider infinite 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin = -1.f, UIMin = -1.f))
	float GameHoursDuration = -1.f;
	
protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionShowScreenText : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ScreenTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SubTitle;
	
	// In seconds
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ShowDuration = 5.f;
	
protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

// Just tell the quest system game mode to do "something" that matches this tag. Useful for scripting game/level events in blueprints
// when it makes no sense to make a dedicated cpp class OR when it makes no sense for the quest system to know about it because it is too game-specific
// for example: in G2VS2, an action to make a player enter a specific queue on a specific level: quest system shouldn't know anything about queues and their interface
// so we just trigger an arbitrary event and then G2VS2's game mode matches a tag to a UObject with an interface IArbitraryGameEvent
// and this UObject is a blueprint which has hardcoded queue id, hardcoded player access, etc
USTRUCT()
struct FQuestActionArbitraryTagAction : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UArbitraryQuestAction> ArbitraryQuestActionClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FGameplayTag> TagParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> FloatParameters;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionNpcDisplayReaction : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PhraseId;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionInitiateDialogueWithNpc : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag OptionalDialogueId;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionNpcStartRealtimeDialogue : public FQuestActionBase
{
	GENERATED_BODY()

public:
	// Dialogue instigator
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag DialogueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDialogueParticipantData> DialogueParticipants;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIncludePlayer = true;
	
protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionSendNpcEvent : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery NpcFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Event;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};

USTRUCT()
struct FQuestActionSetSublevelState : public FQuestActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SublevelId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bLoaded = true;

	// Useful to prevent sublevel to load/unload in front of player which can be deduced if player has some location tag on them
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery PostponeIfPlayerHasTags;

protected:
	virtual void ExecuteInternal(const FQuestSystemContext& Context) const override;
};