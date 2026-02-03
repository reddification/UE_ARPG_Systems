#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemiesCoordinatorComponent.generated.h"

class ACharacter;
class UCoordinatorQueryHelper;

// sorted by priority
UENUM(BlueprintType)
enum class ENpcCombatRole : uint8
{
	None = 0,
	Attacker = 1,
	Surrounder = 2,
	Idle = 3,
	Max = Idle UMETA(Hidden)
};

/*
 * Coordinator assigns and updates combat roles for registered enemies
 * combat roles represent "layers" of enemies with possible gaps which enables having evenly distributed "circles" of enemies
 * */
UCLASS(BlueprintType, Blueprintable)
class ARPGAI_API UEnemiesCoordinatorComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	struct FNpcCombatRoleContainer
	{
		FNpcCombatRoleContainer() = default;
		FNpcCombatRoleContainer(int InMaxMobs);

		void AddMember(APawn* Character, int Index);

		void RemoveMember(APawn* Character);
		void RemoveMember(int Index);

		void RecountMembers();

		int MaxEnemies = 0;
		int CurrentEnemiesCount = 0;
	
		// this array can contain empty elements. the idea is there's a fix amount of enemies on a circle surrounding target
		// and each index corresponds to a certain angle relative to target.
		// so array can have 5 spots, 0 -> 0 degrees, 1 -> 60 degrees, 2 -> 120 degrees, 3 -> 180 degrees, 4 -> 240 degrees, 5 -> 300 degrees 
		// but only 1, 2 and 4 can be occupied by actual valid pawns while others will be nullptr
		TArray<TWeakObjectPtr<APawn>> Enemies;
	};
	
	struct FPendingRoleTransfer
	{
		FPendingRoleTransfer(ENpcCombatRole FromRole, int Index, ENpcCombatRole ToRole)
			: FromRole(FromRole), Index(Index), ToRole(ToRole)
		{
		}

		ENpcCombatRole FromRole = ENpcCombatRole::None;
		int Index = -1;
		ENpcCombatRole ToRole = ENpcCombatRole::None;
	};
	
	
public:
	UEnemiesCoordinatorComponent();
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	ENpcCombatRole RegisterEnemy(APawn* Enemy);
	void UnregisterEnemy(APawn* MobCharacter);
	bool IsEnemyRegistered(APawn* MobCharacter) const;
	bool GetEnemyDesiredPosition(APawn* Enemy, float& AvgAngle) const;
	ENpcCombatRole GetEnemyRole(APawn* MobCharacter) const;
	bool TrySetNpcSquadRole(APawn* EnemyPawn, ENpcCombatRole NewMobSquadRole);
	int GetAttackersCount() const;
	int GetAttackersCount(const AActor* ExceptActor) const;

	const TArray<TWeakObjectPtr<APawn>>* GetMembers(ENpcCombatRole NpcRole) const;
	TArray<APawn*> GetEnemies(ENpcCombatRole Role) const;

private:
	void UpdateRegisteredEnemies();
	void AddToContainers(APawn* Enemy, ENpcCombatRole CombatRole);
	bool RemoveFromContainers(APawn* MobCharacter);
	void CleanNonValidEnemies();
	int GetAvailableIndexOfEnemyCombatRole(ENpcCombatRole MobSquadRole) const;
	bool ValidateNewEnemyCombatRole(ENpcCombatRole CurrentMobSquadRole, ENpcCombatRole NewMobSquadRole) const;
	
	TMap<TWeakObjectPtr<APawn>, ENpcCombatRole> RegisteredEnemies;
	TMap<ENpcCombatRole, FNpcCombatRoleContainer> CombatRoleContainers;
};
