// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardAssetProvider.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Components/ActorComponent.h"

#include "NpcSpawnerComponent.generated.h"

struct FGameplayTag;

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcSpawnDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Character.Id,G2VS2.Character.Id"))
	FGameplayTag NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MakeEditWidget))
	FVector	SpawnRelativeLocation = FVector::ZeroVector;

	// NPCs with same squad id will be registered in a squad.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGuid SquadId;	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcSpawnerComponent : public USceneComponent, public IBlackboardAssetProvider
{
	GENERATED_BODY()

public:
	UNpcSpawnerComponent();

	UFUNCTION(BlueprintCallable)
	void TriggerSpawn();

	void SpawnNpcs();
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// limited by the total number of spawn descriptors
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSpawnerEnabled = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	TArray<FNpcSpawnDescriptor> NpcSpawnDescriptors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	bool bSpawnImmediately = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	bool bDestroyAliveNpcsOnRespawnRequested = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	bool bRespawnAfterAllNpcsKilled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	bool bRegisterAsDesignatedZoneForNpc = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, EditCondition="bRespawnAfterAllNpcsKilled && bSpawnerEnabled"))
	float RespawnAfterAllNpcsKilledDelayGameHours = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1, EditCondition=bSpawnerEnabled))
	int MaxSpawnsPerTick = 3;

	// limited by the total number of spawn descriptors
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1, EditCondition=bSpawnerEnabled))
	int NpcsToSpawnPerActivation = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	FGameplayTagContainer SpawnNpcWithTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	FGameplayTag InitialNpcActivity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	TSoftObjectPtr<UBlackboardData> NpcBlackboard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition=bSpawnerEnabled))
	TMap<FGameplayTag, float> NpcFloatBlackboardParameters;

	// I want to use this with GameplayMessageRouter, but at the same time, the ARPGAI plugin shouldn't be dependent on other plugins
	// So I guess the core game itself will have to have some actor (or subsystem or even game mode) to register GMS listeners for these spawn triggers  
	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// FGameplayTagContainer SpawnTriggers;
	
	bool bSpawnedOnStart = false; // to prevent multiple spawns on spawner loading-unloading by world partition streaming
	TArray<FNpcSpawnDescriptor> PendingNpcSpawns;
	
	TArray<TWeakObjectPtr<AActor>> AliveNpcs;

	void AddNpc(AActor* Npc);

private:
	void OnNpcDied(AActor* Actor);

public: // IBlackboardAssetProvider
	virtual UBlackboardData* GetBlackboardAsset() const override;
};
