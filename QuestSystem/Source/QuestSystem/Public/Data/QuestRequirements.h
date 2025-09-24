#pragma once
#include "QuestEnums.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "QuestRequirements.generated.h"

struct FQuestSystemContext;

USTRUCT(BlueprintType)
struct FQuestRequirementQuestFilter
{
	GENERATED_BODY()
	
	// Any or each of the list
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EContainmentType FilterType;

	// In which state must quests be
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestState QuestState;
	
	// List of quests DTRs
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDataTableRowHandle> QuestsDTRs;

	// Must posses for filter to work or the opposite
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bMustPossess = true;
};

UENUM()
enum class EItemRequirementLogicalExpression : uint8
{
	Equal,
	Greater,
	Less,
	NotEqual
};

USTRUCT(BlueprintType)
struct FQuestItemRequirementExpression
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemRequirementLogicalExpression ItemRequirementLogicalExpression = EItemRequirementLogicalExpression::Equal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery WithTagsFilter;
};

USTRUCT(BlueprintType)
struct FQuestRequirementItemFilter
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ItemsRequirementTagQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FQuestItemRequirementExpression> CountedItemsRequirements;
};

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FQuestRequirementBase
{
	GENERATED_BODY()

public:
	virtual ~FQuestRequirementBase() = default;

	// This one is just game designers to give quick description of what this requirement is about
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ApplicableAtWorldState;

	bool IsApplicable(const FQuestSystemContext& QuestSystemContext) const;
	
	virtual bool IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const { return true; };
};

USTRUCT()
struct QUESTSYSTEM_API FQuestRequirementWorldState : public FQuestRequirementBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery RequiresWorldState;
	
	virtual bool IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const override;
};

USTRUCT()
struct QUESTSYSTEM_API FQuestRequirementCharacterState : public FQuestRequirementBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery RequiresCharacterGameplayState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bPlayer = true;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="!bPlayer"))
	FGameplayTag CharacterId;
	
	virtual bool IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const override;
};

USTRUCT()
struct QUESTSYSTEM_API FQuestRequirementCharacterInventory : public FQuestRequirementBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FQuestRequirementItemFilter ItemFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bPlayer = true;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="!bPlayer"))
	FGameplayTag CharacterId;
	
	virtual bool IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const override;
};

USTRUCT()
struct QUESTSYSTEM_API FQuestRequirementPlayerInProximityOfNpc : public FQuestRequirementBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcId;

	// Visibility is traced from MainCharacter head to TestCharacter head
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bTraceVisibility = true;

	// Distance within which characters must be in order for the requirement to pass
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Proximity = 350.f;	
	
	virtual bool IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const override;
};