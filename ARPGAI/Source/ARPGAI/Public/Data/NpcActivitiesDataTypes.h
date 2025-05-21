#pragma once

#include "GameplayTagContainer.h"
#include "NpcGoalItem.h"
#include "NavFilters/NavigationQueryFilter.h"

#include "NpcActivitiesDataTypes.generated.h"

class UNpcGoalBase;

USTRUCT(BlueprintType)
struct FNpcGoalChain
{
	GENERATED_BODY()

	FNpcGoalChain();
	
	// This field is needed only to set description in editor to quickly understand what this goal chain is about
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Description;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FNpcGoalItem> NpcGoals;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int DesiredFollowers = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="DesiredFollowers > 0"))
	FGameplayTagContainer SuitableSquadMembersIds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="DesiredFollowers > 0"))
	FGameplayTagQuery SuitableSquadMembersFilter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="DesiredFollowers > 0"))
	FGameplayTag SquadMemberAttitudePreset;
};

inline FNpcGoalChain::FNpcGoalChain()
{
	// TODO override GetDescription for FNpcGoalItem and UNpcGoal 
	// for (const auto& NpcGoal : NpcGoals)
	// {
	// 	Description = Description.Append(NpcGoal.GetDescription());
	// 	Description = Description.Append("\n");
	// }
}

UCLASS()
class ARPGAI_API UNpcActivityDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UNavigationQueryFilter> ActivityNavigationFilterClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bPickRandomGoalChain = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FNpcGoalChain> NpcGoalsChains;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag DefaultActivityNpcState;
};

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcActivityOption
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery OnlyAtWorldState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery OnlyAtCharacterState;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNpcActivityDataAsset* ActivityDataAsset;
};

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcActivityParametersOptions
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FNpcActivityOption> NpcActivityOptions;
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
};

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcGoalParameters_Patrol : public FNpcGoalParametersBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PatrolRouteId;
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