#include "BehaviorTree/Services/BTService_RegisterInCoordinator.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/EnemiesCoordinatorComponent.h"
#include "Components/NpcCombatLogicComponent.h"

UBTService_RegisterInCoordinator::UBTService_RegisterInCoordinator()
{
	NodeName = "Register in coordinator";
	TargetActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RegisterInCoordinator, TargetActorBBKey), AActor::StaticClass());
	OutInitialMobSquadRoleTypeBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RegisterInCoordinator, OutInitialMobSquadRoleTypeBBKey),
		StaticEnum<ENpcSquadRole>());
	bNotifyBecomeRelevant = 1;
	bNotifyCeaseRelevant = 1;
	bNotifyTick = 0;
}

uint16 UBTService_RegisterInCoordinator::GetInstanceMemorySize() const
{
	return sizeof(FRegisterInCoordinatorMemory);
}

FString UBTService_RegisterInCoordinator::GetStaticDescription() const
{
	return FString::Printf(TEXT("Target BB: %s\nInitial behavior BB: %s"),
		*TargetActorBBKey.SelectedKeyName.ToString(), *OutInitialMobSquadRoleTypeBBKey.SelectedKeyName.ToString());
}

void UBTService_RegisterInCoordinator::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	const AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorBBKey.SelectedKeyName));
	if (TargetActor == nullptr)
	{
		return;
	}

	UEnemiesCoordinatorComponent* CoordinatorComponent = TargetActor->FindComponentByClass<UEnemiesCoordinatorComponent>();
	if (!ensure(CoordinatorComponent != nullptr))
	{
		return;
	}

	FRegisterInCoordinatorMemory* Memory = CastInstanceNodeMemory<FRegisterInCoordinatorMemory>(NodeMemory);
	Memory->Coordinator = CoordinatorComponent;
	TrySetCoordinatorRegistration(CoordinatorComponent, OwnerComp.GetAIOwner()->GetPawn(), BlackboardComponent, true);
}

void UBTService_RegisterInCoordinator::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FRegisterInCoordinatorMemory* Memory = CastInstanceNodeMemory<FRegisterInCoordinatorMemory>(NodeMemory);
	if (Memory->Coordinator.IsValid() == false)
	{
		return;
	}

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	TrySetCoordinatorRegistration(Memory->Coordinator.Get(), OwnerComp.GetAIOwner()->GetPawn(), BlackboardComponent, false);
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_RegisterInCoordinator::TrySetCoordinatorRegistration(UEnemiesCoordinatorComponent* CoordinatorComponent, APawn* BotCharacter,
	UBlackboardComponent* BlackboardComponent, bool bRegister) const
{
	if (!BotCharacter) return;
	
	auto NpcCombatComponent = BotCharacter->FindComponentByClass<UNpcCombatLogicComponent>();
	if (bRegister)
	{
		ENpcSquadRole NpcAttackRole;
		if (CoordinatorComponent->RegisterNpc(BotCharacter, NpcAttackRole))
		{
			ensure(NpcAttackRole != ENpcSquadRole::None);
			NpcCombatComponent->SetAttackerRole(NpcAttackRole);
			BlackboardComponent->SetValueAsEnum(OutInitialMobSquadRoleTypeBBKey.SelectedKeyName, (uint8)NpcAttackRole);
		}
	}
	else
	{
		CoordinatorComponent->UnregisterMob(BotCharacter);
		BlackboardComponent->ClearValue(OutInitialMobSquadRoleTypeBBKey.SelectedKeyName);
		NpcCombatComponent->SetAttackerRole(ENpcSquadRole::None);
	}
}

