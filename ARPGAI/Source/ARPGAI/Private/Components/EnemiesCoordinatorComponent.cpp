

#include "Components/EnemiesCoordinatorComponent.h"

#include "Settings/NpcCombatSettings.h"
#include "TimerManager.h"


UEnemiesCoordinatorComponent::UEnemiesCoordinatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemiesCoordinatorComponent::BeginPlay()
{
	Super::BeginPlay();
	const UNpcCombatSettings* Settings = GetDefault<UNpcCombatSettings>();

	EnemyRoleToEnemies.Add(ENpcSquadRole::Attacker, FNpcSquadRoleData(Settings->MaxAttackers));
	EnemyRoleToEnemies.Add(ENpcSquadRole::Surrounder, FNpcSquadRoleData(Settings->MaxTaunters));
	//Currently not used
	EnemyRoleToEnemies.Add(ENpcSquadRole::None, FNpcSquadRoleData(0));
	float UpdateRate = GetDefault<UNpcCombatSettings>()->MobCoordinatorUpdateRate;
	GetWorld()->GetTimerManager().SetTimer(TimerHandleUpdateMobs, this, &UEnemiesCoordinatorComponent::UpdateRegisteredMobs, UpdateRate, true);
}

void UEnemiesCoordinatorComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

bool UEnemiesCoordinatorComponent::RegisterNpc(APawn* Enemy, ENpcSquadRole& InitialMobSquadRole)
{
	UpdateRegisteredMobs();
	// Check current condition of arrays, update with new mob, assign MobSquadRole
	if (IsEnemyRegistered(Enemy) == true)
	{
		return false;
	}

	ENpcSquadRole SelectedRole;
	if (EnemyRoleToEnemies[ENpcSquadRole::Attacker].CurrentMobsCount < EnemyRoleToEnemies[ENpcSquadRole::Attacker].MaxEnemies)
	{
		SelectedRole = ENpcSquadRole::Attacker;
	}
	else //EMobSquadRole::Taunter
	{
		SelectedRole = ENpcSquadRole::Surrounder;
	}
	AddToContainers(Enemy, SelectedRole);
	InitialMobSquadRole = SelectedRole;
	return true;
}

void UEnemiesCoordinatorComponent::UnregisterMob(const APawn* MobCharacter)
{
	if (IsEnemyRegistered(MobCharacter) == false)
	{
		return;
	}
	check(RemoveFromContainers(MobCharacter));
}


bool UEnemiesCoordinatorComponent::IsEnemyRegistered(const APawn* MobCharacter) const
{
	return RegisteredEnemies.Contains(MobCharacter);
}

void UEnemiesCoordinatorComponent::UpdateRegisteredMobs()
{
	CleanNonValidMobs();

	for (const auto& Enemy : EnemyRoleToEnemies[ENpcSquadRole::Surrounder].Enemies)
	{
		if (Enemy.IsValid())
		{
			TrySetNpcSquadRole(Enemy.Get(), ENpcSquadRole::Attacker);
		}
	}
}

void UEnemiesCoordinatorComponent::CleanNonValidMobs()
{
	TArray<const APawn*> TempNonValidArray;
	for (const auto& Pair : RegisteredEnemies)
	{
		if (!Pair.Key.IsValid())
		{
			TempNonValidArray.Add(Pair.Key.Get());
		}
	}
	if (TempNonValidArray.Num() == 0)
	{
		return;
	}
	for (const APawn* NonValidMob : TempNonValidArray)
	{
		RegisteredEnemies.Remove(NonValidMob);
	}

	for (auto& Pair : EnemyRoleToEnemies)
	{
		Pair.Value.RecountMembers();
	}
}

void UEnemiesCoordinatorComponent::AddToContainers(APawn* Enemy, ENpcSquadRole MobSquadRole)
{
	RegisteredEnemies.Add(Enemy, MobSquadRole);
	int NonValidMobIndx = GetAvailableIndexOfMobSquadRole(MobSquadRole);
	if (NonValidMobIndx == INDEX_NONE)
	{
		return;
	}
	
	EnemyRoleToEnemies[MobSquadRole].AddMember(Enemy, NonValidMobIndx);
}


bool UEnemiesCoordinatorComponent::TrySetNpcSquadRole(APawn* EnemyPawn, ENpcSquadRole NewMobSquadRole)
{
	const ENpcSquadRole CurrentMobSquadRole = GetNpcSquadRole(EnemyPawn);
	if (CurrentMobSquadRole == NewMobSquadRole)
	{
		return true;
	}

	if (ValidateNewMobSquadRole(CurrentMobSquadRole, NewMobSquadRole) == false)
	{
		return false;
	}

	const int EmptyIndex = GetAvailableIndexOfMobSquadRole(NewMobSquadRole);
	if (EmptyIndex == INDEX_NONE)
	{
		return false;
	}

	const int CurrentIndex = EnemyRoleToEnemies[CurrentMobSquadRole].Enemies.IndexOfByKey(EnemyPawn);
	if (CurrentIndex == INDEX_NONE)
	{
		return false;
	}

	EnemyRoleToEnemies[CurrentMobSquadRole].RemoveMember(CurrentIndex);

	RegisteredEnemies[EnemyPawn] = NewMobSquadRole;
	EnemyRoleToEnemies[NewMobSquadRole].AddMember(EnemyPawn, EmptyIndex);
	return true;
}

int UEnemiesCoordinatorComponent::GetAttackersCount() const
{
	return EnemyRoleToEnemies[ENpcSquadRole::Attacker].CurrentMobsCount;
}

int UEnemiesCoordinatorComponent::GetAttackersCount(const AActor* ExceptActor) const
{
	int Result = EnemyRoleToEnemies[ENpcSquadRole::Attacker].CurrentMobsCount;
	if (EnemyRoleToEnemies[ENpcSquadRole::Attacker].Enemies.Contains(ExceptActor))
	{
		Result--;
	}
	
	return Result;
}

const TArray<TWeakObjectPtr<APawn>>* UEnemiesCoordinatorComponent::GetMembers(ENpcSquadRole NpcRole) const
{
	return &EnemyRoleToEnemies[NpcRole].Enemies;
}

bool UEnemiesCoordinatorComponent::RemoveFromContainers(const APawn* MobCharacter)
{
	const ENpcSquadRole MobSquadRole = RegisteredEnemies.FindAndRemoveChecked(MobCharacter);
	int FoundMobIndx = EnemyRoleToEnemies[MobSquadRole].Enemies.IndexOfByKey(MobCharacter);
	if (FoundMobIndx != INDEX_NONE)
	{
		EnemyRoleToEnemies[MobSquadRole].RemoveMember(FoundMobIndx);
	}
	return true;
}

int UEnemiesCoordinatorComponent::GetAvailableIndexOfMobSquadRole(ENpcSquadRole MobSquadRole) const
{
	int MaxGapStartIndex = 0;
	int MaxGapSize = 0;
	int CurrentGapSize = 0;
	int MaxNum = EnemyRoleToEnemies[MobSquadRole].MaxEnemies;
	if (EnemyRoleToEnemies[MobSquadRole].CurrentMobsCount <= 0)
		return 0;
	
	int MaxGapEndIndex = MaxNum;
	const auto& MobArray = EnemyRoleToEnemies[MobSquadRole].Enemies;

	for (int i = 0; i < MaxNum; i++)
	{
		if (MobArray[i].IsValid() == false)
		{
			CurrentGapSize++;
		}
		else
		{
			if (CurrentGapSize > MaxGapSize)
			{
				MaxGapSize = CurrentGapSize;
				MaxGapEndIndex = i;
				MaxGapStartIndex = i - CurrentGapSize;
			}

			CurrentGapSize = 0;
		}
	}

	if (CurrentGapSize > MaxGapSize)
	{
		MaxGapSize = CurrentGapSize;
		MaxGapEndIndex = MaxNum;
		MaxGapStartIndex = MaxNum - CurrentGapSize;
	}

	const int ResultIndex = MaxGapSize > 0 ? MaxGapStartIndex + ((MaxGapEndIndex - MaxGapStartIndex) / 2) : INDEX_NONE;

	return ResultIndex;
}

bool UEnemiesCoordinatorComponent::GetEnemyDesiredPosition(const APawn* Enemy, float& AvgAngle) const
{
	if (!IsEnemyRegistered(Enemy))
		return false;
	
	const ENpcSquadRole EnemyRole = RegisteredEnemies[Enemy];
	const int Segment = EnemyRoleToEnemies[EnemyRole].Enemies.IndexOfByKey(Enemy);
	const int MaxEnemies = EnemyRoleToEnemies[EnemyRole].MaxEnemies;
	if (Segment == INDEX_NONE)
		return false;

	AvgAngle = 360.f / MaxEnemies * Segment;
	return true;
}


ENpcSquadRole UEnemiesCoordinatorComponent::GetNpcSquadRole(const APawn* MobCharacter) const
{
	const ENpcSquadRole* MobSquadRole = RegisteredEnemies.Find(MobCharacter);
	check(MobSquadRole);
	return *MobSquadRole;
}

bool UEnemiesCoordinatorComponent::ValidateNewMobSquadRole(ENpcSquadRole CurrentMobSquadRole, ENpcSquadRole NewMobSquadRole) const
{
	if (CurrentMobSquadRole == ENpcSquadRole::Attacker && NewMobSquadRole == ENpcSquadRole::Surrounder)
	{
		for (const auto& Taunter : EnemyRoleToEnemies[ENpcSquadRole::Surrounder].Enemies)
		{
			if (Taunter.IsValid())
			{
				return true;
			}
		}

		return false;
	}

	return true;
}

FNpcSquadRoleData::FNpcSquadRoleData(int InMaxMobs): MaxEnemies(InMaxMobs)
{
	Enemies.SetNum(InMaxMobs);
}

void FNpcSquadRoleData::AddMember(APawn* Character, int Index)
{
	Enemies[Index] = Character;
	CurrentMobsCount++;
}

void FNpcSquadRoleData::RemoveMember(int Index)
{
	Enemies[Index].Reset();
	CurrentMobsCount--;
}

void FNpcSquadRoleData::RecountMembers()
{
	CurrentMobsCount = 0;
	for (const auto& Mob : Enemies)
	{
		if (Mob.IsValid())
		{
			CurrentMobsCount++;
		}
	}
}
