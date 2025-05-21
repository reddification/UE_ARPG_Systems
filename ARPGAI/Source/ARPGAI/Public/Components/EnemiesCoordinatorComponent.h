#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemiesCoordinatorComponent.generated.h"

class ACharacter;
class UCoordinatorQueryHelper;

UENUM(BlueprintType)
enum class ENpcSquadRole : uint8
{
	None = 0,
	Surrounder = 1,
	Attacker = 2,
	Follower = 3
};

struct FNpcSquadRoleData
{
	FNpcSquadRoleData() = default;
	FNpcSquadRoleData(int InMaxMobs);

	void AddMember(APawn* Character, int Index);

	void RemoveMember(int Index);

	void RecountMembers();

	int MaxEnemies = 0;
	int CurrentMobsCount = 0;
	
	TArray<TWeakObjectPtr<APawn>> Enemies;
};

/* Coordinator manages mobs attacking it's player owner, assigns them behaviour and provides their segment around player for EQS.
 * Currently juggles taunters and attackers.
 * Reserves arrays of active mobs pointers with this MobSquadRole, sets pointer to a mob if it has gained this MobSquadRole, invalidates pointer if mob lost this MobSquadRole.
 * */
UCLASS(BlueprintType, Blueprintable)
class ARPGAI_API UEnemiesCoordinatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemiesCoordinatorComponent();

	bool RegisterNpc(APawn* Enemy, ENpcSquadRole& InitialMobSquadRole);
	void UnregisterMob(const APawn* MobCharacter);
	bool IsEnemyRegistered(const APawn* MobCharacter) const;
	bool GetEnemyDesiredPosition(const APawn* Enemy, float& AvgAngle) const;
	ENpcSquadRole GetNpcSquadRole(const APawn* MobCharacter) const;
	bool TrySetNpcSquadRole(APawn* EnemyPawn, ENpcSquadRole NewMobSquadRole);
	int GetAttackersCount() const;
	int GetAttackersCount(const AActor* ExceptActor) const;

	const TArray<TWeakObjectPtr<APawn>>* GetMembers(ENpcSquadRole NpcRole) const;

protected:
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

private:
	void UpdateRegisteredMobs();
	TMap<TWeakObjectPtr<const APawn>, ENpcSquadRole> RegisteredEnemies;
	TMap<ENpcSquadRole, FNpcSquadRoleData> EnemyRoleToEnemies;
	void AddToContainers(APawn* Enemy, ENpcSquadRole MobSquadRole);
	bool RemoveFromContainers(const APawn* MobCharacter);
	void CleanNonValidMobs();
	int GetAvailableIndexOfMobSquadRole(ENpcSquadRole MobSquadRole) const;
	bool ValidateNewMobSquadRole(ENpcSquadRole CurrentMobSquadRole, ENpcSquadRole NewMobSquadRole) const;

	UPROPERTY()
	FTimerHandle TimerHandleUpdateMobs;
};
