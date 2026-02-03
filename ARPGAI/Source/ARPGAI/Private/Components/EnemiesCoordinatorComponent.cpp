#include "Components/EnemiesCoordinatorComponent.h"

#include "Settings/NpcCombatSettings.h"
#include "TimerManager.h"

UEnemiesCoordinatorComponent::UEnemiesCoordinatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 1.f;
	bWantsInitializeComponent = true;
}

void UEnemiesCoordinatorComponent::InitializeComponent()
{
	Super::InitializeComponent();
	const UNpcCombatSettings* Settings = GetDefault<UNpcCombatSettings>();
	if (ensure(Settings))
	{
		CombatRoleContainers.Add(ENpcCombatRole::None, FNpcCombatRoleContainer(0));
		CombatRoleContainers.Add(ENpcCombatRole::Attacker, FNpcCombatRoleContainer(Settings->MaxAttackers));
		CombatRoleContainers.Add(ENpcCombatRole::Surrounder, FNpcCombatRoleContainer(Settings->MaxSurrounders));
		CombatRoleContainers.Add(ENpcCombatRole::Idle, FNpcCombatRoleContainer(50));
	}
}

void UEnemiesCoordinatorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateRegisteredEnemies();
}


ENpcCombatRole UEnemiesCoordinatorComponent::RegisterEnemy(APawn* Enemy)
{
	UpdateRegisteredEnemies();
	if (IsEnemyRegistered(Enemy))
		return RegisteredEnemies[Enemy];

	int MaxRoles = static_cast<int>(ENpcCombatRole::Max);
	ENpcCombatRole SelectedRole = ENpcCombatRole::None;
	for (int i = 1; i < MaxRoles; i++)
	{
		ENpcCombatRole IteratorRole = static_cast<ENpcCombatRole>(i);
		if (CombatRoleContainers[IteratorRole].CurrentEnemiesCount < CombatRoleContainers[IteratorRole].MaxEnemies)
		{
			SelectedRole = IteratorRole;
			break;	
		}
	}
	
	if (SelectedRole == ENpcCombatRole::None)
		return SelectedRole;
	
	AddToContainers(Enemy, SelectedRole);
	
	if (RegisteredEnemies.Num() == 1)
		SetComponentTickEnabled(true);
	
	return SelectedRole;
}

void UEnemiesCoordinatorComponent::UnregisterEnemy(APawn* MobCharacter)
{
	if (IsEnemyRegistered(MobCharacter) == false)
		return;
	
	RemoveFromContainers(MobCharacter);
	if (RegisteredEnemies.Num() == 0)
		SetComponentTickEnabled(false);
}


bool UEnemiesCoordinatorComponent::IsEnemyRegistered(APawn* MobCharacter) const
{
	return RegisteredEnemies.Contains(MobCharacter);
}

void UEnemiesCoordinatorComponent::UpdateRegisteredEnemies()
{
	CleanNonValidEnemies();
	if (RegisteredEnemies.IsEmpty())
		return;
	
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	TArray<TPair<APawn*, float>> DistanceSortedEnemies;
	for (const auto& RegisteredEnemy : RegisteredEnemies)
		DistanceSortedEnemies.Add({ RegisteredEnemy.Key.Get(), (RegisteredEnemy.Key->GetActorLocation() - OwnerLocation).SizeSquared() });
	
	DistanceSortedEnemies.Sort([](const TPair<APawn*, float>& A, const TPair<APawn*, float>& B) { return A.Value < B.Value; });
	
	// TODO 31.01.2026 (aki): consider doing a more optimal algorithm. i don't know how, but I feel like it's possible   
	int MaxRoles = static_cast<int>(ENpcCombatRole::Max);
	TMap<ENpcCombatRole, TArray<APawn*>> NewCombatRoleContainers;
	
	int EnemyIndex = 0;
	for (int RoleIndex = 1; RoleIndex < MaxRoles && EnemyIndex < DistanceSortedEnemies.Num(); RoleIndex++)
	{
		ENpcCombatRole CurrentIteratorRole = static_cast<ENpcCombatRole>(RoleIndex);
		const int RoleMembersCount = CombatRoleContainers[CurrentIteratorRole].MaxEnemies;
		NewCombatRoleContainers.Add(CurrentIteratorRole);
		NewCombatRoleContainers[CurrentIteratorRole].Reserve(RoleMembersCount);
		for (int j = 0; j < RoleMembersCount && EnemyIndex < DistanceSortedEnemies.Num(); j++, EnemyIndex++)
		{
			auto DistanceSortedPawn = DistanceSortedEnemies[EnemyIndex].Key;
			NewCombatRoleContainers[CurrentIteratorRole].Add(DistanceSortedPawn);
			// NewEnemiesToRoles.Add(DistanceSortedPawn, CurrentIteratorRole);
			ENpcCombatRole EnemyCurrentRole = RegisteredEnemies[DistanceSortedEnemies[EnemyIndex].Key];
			if (EnemyCurrentRole != CurrentIteratorRole)
				CombatRoleContainers[EnemyCurrentRole].RemoveMember(DistanceSortedEnemies[EnemyIndex].Key);
		}
	}
	
	if (EnemyIndex < DistanceSortedEnemies.Num() - 1)
	{
		// wtf? i guess it means they didn't fit in any combat role.
		// but then again, in this case they should have never gotten into registered enemies
		ensure(false);
		
		for (int i = EnemyIndex; i < DistanceSortedEnemies.Num(); i++)
		{
			auto RolePtr = RegisteredEnemies.Find(DistanceSortedEnemies[i].Key);
			if (ensure(RolePtr))
			{
				CombatRoleContainers[*RolePtr].RemoveMember(DistanceSortedEnemies[i].Key);
				RegisteredEnemies.Remove(DistanceSortedEnemies[i].Key);
			}
		}
	}
	
	for (const auto& NewCombatRoleContainer : NewCombatRoleContainers)
	{
		for (auto* Pawn : NewCombatRoleContainer.Value)
		{
			if (NewCombatRoleContainer.Key != RegisteredEnemies[Pawn])
			{
				int NewIndex = GetAvailableIndexOfEnemyCombatRole(NewCombatRoleContainer.Key);
				if (ensure(NewIndex != INDEX_NONE))
				{
					RegisteredEnemies[Pawn] = NewCombatRoleContainer.Key;
					CombatRoleContainers[NewCombatRoleContainer.Key].AddMember(Pawn, NewIndex);
				}
			}
		}
	}
}

void UEnemiesCoordinatorComponent::CleanNonValidEnemies()
{
	TArray<const TWeakObjectPtr<APawn>*> TempNonValidArray;
	for (const auto& Pair : RegisteredEnemies)
		if (!Pair.Key.IsValid())
			TempNonValidArray.Add(&Pair.Key);
	
	if (TempNonValidArray.Num() == 0)
		return;

	for (const auto NonValidMob : TempNonValidArray)
		RegisteredEnemies.Remove(*NonValidMob);
	
	for (auto& Pair : CombatRoleContainers)
		Pair.Value.RecountMembers();
	
	if (RegisteredEnemies.IsEmpty())
		SetComponentTickEnabled(false);
}

void UEnemiesCoordinatorComponent::AddToContainers(APawn* Enemy, ENpcCombatRole CombatRole)
{
	RegisteredEnemies.Add(Enemy, CombatRole);
	int NewEnemyIndex = GetAvailableIndexOfEnemyCombatRole(CombatRole);
	if (NewEnemyIndex == INDEX_NONE)
		return;
	
	CombatRoleContainers[CombatRole].AddMember(Enemy, NewEnemyIndex);
}

bool UEnemiesCoordinatorComponent::TrySetNpcSquadRole(APawn* EnemyPawn, ENpcCombatRole NewMobSquadRole)
{
	const ENpcCombatRole CurrentMobSquadRole = GetEnemyRole(EnemyPawn);
	if (CurrentMobSquadRole == NewMobSquadRole)
		return true;

	if (ValidateNewEnemyCombatRole(CurrentMobSquadRole, NewMobSquadRole) == false)
		return false;

	const int EmptyIndex = GetAvailableIndexOfEnemyCombatRole(NewMobSquadRole);
	if (EmptyIndex == INDEX_NONE)
		return false;

	const int CurrentIndex = CombatRoleContainers[CurrentMobSquadRole].Enemies.IndexOfByKey(EnemyPawn);
	if (CurrentIndex == INDEX_NONE)
		return false;

	CombatRoleContainers[CurrentMobSquadRole].RemoveMember(CurrentIndex);
	RegisteredEnemies[EnemyPawn] = NewMobSquadRole;
	CombatRoleContainers[NewMobSquadRole].AddMember(EnemyPawn, EmptyIndex);
	return true;
}

int UEnemiesCoordinatorComponent::GetAttackersCount() const
{
	return CombatRoleContainers[ENpcCombatRole::Attacker].CurrentEnemiesCount;
}

int UEnemiesCoordinatorComponent::GetAttackersCount(const AActor* ExceptActor) const
{
	int Result = CombatRoleContainers[ENpcCombatRole::Attacker].CurrentEnemiesCount;
	if (CombatRoleContainers[ENpcCombatRole::Attacker].Enemies.Contains(ExceptActor))
		Result--;
	
	return Result;
}

const TArray<TWeakObjectPtr<APawn>>* UEnemiesCoordinatorComponent::GetMembers(ENpcCombatRole NpcRole) const
{
	return &CombatRoleContainers[NpcRole].Enemies;
}

TArray<APawn*> UEnemiesCoordinatorComponent::GetEnemies(ENpcCombatRole Role) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UEnemiesCoordinatorComponent::GetEnemies)
	
	if (!CombatRoleContainers.Contains(Role))
		return TArray<APawn*>();
	
	TArray<APawn*> Result;
	Result.Reserve(CombatRoleContainers[Role].MaxEnemies);
	for (int i = 0; i < CombatRoleContainers[Role].Enemies.Num(); i++)
		if (CombatRoleContainers[Role].Enemies[i].IsValid())
			Result.Add(CombatRoleContainers[Role].Enemies[i].Get());
	
	return Result;
}

bool UEnemiesCoordinatorComponent::RemoveFromContainers(APawn* MobCharacter)
{
	const ENpcCombatRole MobSquadRole = RegisteredEnemies.FindAndRemoveChecked(MobCharacter);
	int FoundMobIndx = CombatRoleContainers[MobSquadRole].Enemies.IndexOfByKey(MobCharacter);
	if (FoundMobIndx != INDEX_NONE)
	{
		CombatRoleContainers[MobSquadRole].RemoveMember(FoundMobIndx);
	}
	return true;
}

int UEnemiesCoordinatorComponent::GetAvailableIndexOfEnemyCombatRole(ENpcCombatRole MobSquadRole) const
{
	int MaxGapStartIndex = 0;
	int MaxGapSize = 0;
	int CurrentGapSize = 0;
	int MaxNum = CombatRoleContainers[MobSquadRole].MaxEnemies;
	if (CombatRoleContainers[MobSquadRole].CurrentEnemiesCount <= 0)
		return 0;
	
	int MaxGapEndIndex = MaxNum;
	const auto& MobArray = CombatRoleContainers[MobSquadRole].Enemies;

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

bool UEnemiesCoordinatorComponent::GetEnemyDesiredPosition(APawn* Enemy, float& AvgAngle) const
{
	if (!IsEnemyRegistered(Enemy))
		return false;
	
	const ENpcCombatRole EnemyRole = RegisteredEnemies[Enemy];
	const int Segment = CombatRoleContainers[EnemyRole].Enemies.IndexOfByKey(Enemy);
	const int MaxEnemies = CombatRoleContainers[EnemyRole].MaxEnemies;
	if (Segment == INDEX_NONE)
		return false;

	AvgAngle = 360.f / MaxEnemies * Segment;
	return true;
}


ENpcCombatRole UEnemiesCoordinatorComponent::GetEnemyRole(APawn* MobCharacter) const
{
	const ENpcCombatRole* NpcCombatRolePtr = RegisteredEnemies.Find(MobCharacter);
	return NpcCombatRolePtr != nullptr ? *NpcCombatRolePtr : ENpcCombatRole::None;
}

bool UEnemiesCoordinatorComponent::ValidateNewEnemyCombatRole(ENpcCombatRole CurrentMobSquadRole, ENpcCombatRole NewMobSquadRole) const
{
	if (CurrentMobSquadRole == ENpcCombatRole::Attacker && NewMobSquadRole == ENpcCombatRole::Surrounder)
	{
		for (const auto& Taunter : CombatRoleContainers[ENpcCombatRole::Surrounder].Enemies)
			if (Taunter.IsValid())
				return true;

		return false;
	}

	return true;
}

UEnemiesCoordinatorComponent::FNpcCombatRoleContainer::FNpcCombatRoleContainer(int InMaxMobs): MaxEnemies(InMaxMobs)
{
	Enemies.SetNum(InMaxMobs);
}

void UEnemiesCoordinatorComponent::FNpcCombatRoleContainer::AddMember(APawn* Character, int Index)
{
	Enemies[Index] = Character;
	CurrentEnemiesCount++;
}

void UEnemiesCoordinatorComponent::FNpcCombatRoleContainer::RemoveMember(APawn* Character)
{
	for (int i = 0; i < Enemies.Num(); i++)
	{
		if (Enemies[i] == Character)
		{
			Enemies[i].Reset();
			CurrentEnemiesCount--;
			return;
		}
	}
}

void UEnemiesCoordinatorComponent::FNpcCombatRoleContainer::RemoveMember(int Index)
{
	Enemies[Index].Reset();
	CurrentEnemiesCount--;
}

// container doesnt remove any elements because enemies positions in container are fixed
void UEnemiesCoordinatorComponent::FNpcCombatRoleContainer::RecountMembers()
{
	CurrentEnemiesCount = 0;
	for (const auto& Mob : Enemies)
		if (Mob.IsValid())
			CurrentEnemiesCount++;
}
