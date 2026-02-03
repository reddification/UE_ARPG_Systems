#pragma once

#include "GameplayTagContainer.h"
#include "SmartObjectTypes.h"
#include "AITypes.h"
#include "CommonWrapperTypes.h"
#include "AiDataTypes.generated.h"

class INpcZone;
class UNpcPatrolRouteComponent;

#define AI_BRAINMESSAGE_FLAG_IMMEDIATE 0x01

UENUM(BlueprintType)
enum class ENpcStates : uint8
{
	Default,
	Combat,
	Dialogue,
	Following    
};

USTRUCT(BlueprintType)
struct FNpcAttitude
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery CharacterTagsAndWorldState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.Attitude"))
	FGameplayTag Attitude;
};

USTRUCT(BlueprintType)
struct FNpcAttitudes
{
	GENERATED_BODY()

	// Order matters. When looking for a matching attribute, complexity of the gameplay tag query condition is not taken into account,
	// only order of appearance is
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FNpcAttitude> NpcAttitudes;
};

/**
 * Data added to a Smart Object slot when interaction is started on it. Allows to look up the user.
 */
USTRUCT(BlueprintType)
struct ARPGAI_API FActivitySmartObjectSlotUserData : public FSmartObjectSlotStateData
{
	GENERATED_BODY()

	FActivitySmartObjectSlotUserData() :Super() {};
	explicit FActivitySmartObjectSlotUserData(APawn* InUserPawn) : User(InUserPawn) {}
		
	TWeakObjectPtr<APawn> User = nullptr;
};

USTRUCT(Blueprintable)
struct FCombatMoveSpeedsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceToTargetToMoveSpeedDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bStrafe = true;	
};

struct FNpcPatrolRouteData
{
	FVector RoutePointLocation = FVector::ZeroVector;
	int RoutePointIndex = -1;
	int InitialPointIndex = -1;
	bool bGoingForward = true;
	bool bCyclic = true;
	TWeakObjectPtr<const UNpcPatrolRouteComponent> PatrolRouteComponent;
	
	// this field is used only to pick the best route and can contain both squared distance from point to querier (NPC) at the moment of search and regular distance, depending on the test type (pathfinding or regular)  
	float Distance = FLT_MAX;

	FORCEINLINE bool IsValid() const
	{
		return RoutePointIndex >= 0 && RoutePointLocation != FVector::ZeroVector && RoutePointLocation != FAISystem::InvalidLocation && PatrolRouteComponent.IsValid();
	}
};

struct FNpcPatrolRouteAdvanceResult
{
	FVector NextLocation = FVector::ZeroVector;
	int LoopCount = 0;
	bool bReachedEdge = false;
	
	FORCEINLINE bool IsValid() const
	{
		return NextLocation != FVector::ZeroVector && NextLocation != FAISystem::InvalidLocation;
	}
};

USTRUCT()
struct ARPGAI_API FTemporaryCharacterAttitudeMemory
{
	GENERATED_BODY()
	
	UPROPERTY()
	FGameplayTag AttitudeTag;

	UPROPERTY()
	FDateTime ValidUntilGameTime = 0.f;

	UPROPERTY()
	bool bShareableWithAllies = false;
};

USTRUCT()
struct FNpcAreasContainer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TScriptInterface<INpcZone>> NpcAreas;
};

UCLASS()
class UNpcStatesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="AI.State,G2VS2.Character.State"))
	TMap<FGameplayTag, FGameplayEffectsWrapper> StateEffects;
};

USTRUCT(BlueprintType)
struct FBehaviorEvaluatorBlockRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag BehaviorEvaluatorTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIndefinitely = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bIndefinitely==false"))
	float Duration = 10.f;
};
