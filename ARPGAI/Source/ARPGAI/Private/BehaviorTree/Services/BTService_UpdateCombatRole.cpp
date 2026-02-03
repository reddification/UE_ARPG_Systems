#include "BehaviorTree/Services/BTService_UpdateCombatRole.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/EnemiesCoordinatorComponent.h"
#include "Components/NpcCombatLogicComponent.h"

UBTService_UpdateCombatRole::UBTService_UpdateCombatRole()
{
	NodeName = "Update Combat Role";
	TargetActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateCombatRole, TargetActorBBKey), AActor::StaticClass());
	OutNpcCombatRoleTypeBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateCombatRole, OutNpcCombatRoleTypeBBKey),
		StaticEnum<ENpcCombatRole>());
	bNotifyBecomeRelevant = 1;
	bNotifyCeaseRelevant = 1;
}

void UBTService_UpdateCombatRole::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	auto TargetChangedObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_UpdateCombatRole::OnTargetChanged);
	BlackboardComponent->RegisterObserver(TargetActorBBKey.GetSelectedKeyID(), this, TargetChangedObserverDelegate);
	
	const AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorBBKey.SelectedKeyName));
	if (TargetActor == nullptr)
		return;

	UEnemiesCoordinatorComponent* CoordinatorComponent = TargetActor->FindComponentByClass<UEnemiesCoordinatorComponent>();
	if (!ensure(CoordinatorComponent != nullptr))
		return;

	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	FBTMemory_UpdateCombatRole* Memory = CastInstanceNodeMemory<FBTMemory_UpdateCombatRole>(NodeMemory);
	Memory->CoordinatorComponent = CoordinatorComponent;
	Memory->CombatLogicComponent = Pawn->FindComponentByClass<UNpcCombatLogicComponent>();
	TrySetCoordinatorRegistration(Memory, Pawn, BlackboardComponent, true);
}

void UBTService_UpdateCombatRole::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackboardComponent))
		return;
	
	FBTMemory_UpdateCombatRole* Memory = CastInstanceNodeMemory<FBTMemory_UpdateCombatRole>(NodeMemory);
	if (Memory->CoordinatorComponent.IsValid())
		TrySetCoordinatorRegistration(Memory, OwnerComp.GetAIOwner()->GetPawn(), BlackboardComponent, false);
	
	if (Memory->CombatLogicComponent.IsValid())
		Memory->CombatLogicComponent->SetCombatRole(ENpcCombatRole::None);
	
	BlackboardComponent->UnregisterObserversFrom(this);
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_UpdateCombatRole::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto BTMemory = reinterpret_cast<FBTMemory_UpdateCombatRole*>(NodeMemory);
	if (BTMemory->CoordinatorComponent.IsValid() && BTMemory->CombatLogicComponent.IsValid())
	{
		auto CurrentRole = BTMemory->CoordinatorComponent->GetEnemyRole(OwnerComp.GetAIOwner()->GetPawn());
		BTMemory->CombatLogicComponent->SetCombatRole(CurrentRole);
		OwnerComp.GetBlackboardComponent()->SetValueAsEnum(OutNpcCombatRoleTypeBBKey.SelectedKeyName, static_cast<uint8>(CurrentRole));
	}
}

EBlackboardNotificationResult UBTService_UpdateCombatRole::OnTargetChanged(const UBlackboardComponent& BlackboardComponent, 
	FBlackboard::FKey Key)
{
	if (!ensure(Key == TargetActorBBKey.GetSelectedKeyID()))
		return EBlackboardNotificationResult::RemoveObserver;
	
	auto OwnerComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	const int32 NodeInstanceIdx = OwnerComp->FindInstanceContainingNode(this);
	FBTMemory_UpdateCombatRole* BTMemory = CastInstanceNodeMemory<FBTMemory_UpdateCombatRole>(OwnerComp->GetNodeMemory(this, NodeInstanceIdx));
	auto NewTarget = Cast<AActor>(BlackboardComponent.GetValueAsObject(TargetActorBBKey.SelectedKeyName));
	auto Pawn = OwnerComp->GetAIOwner()->GetPawn();
	ENpcCombatRole CombatRole = ENpcCombatRole::None;
	if (BTMemory->CoordinatorComponent.IsValid() && ensure(BTMemory->CoordinatorComponent->GetOwner() != NewTarget))
	{
		BTMemory->CoordinatorComponent->UnregisterEnemy(Pawn);
		BTMemory->CoordinatorComponent.Reset();
	}
	
	if (NewTarget != nullptr)
	{
		BTMemory->CoordinatorComponent = NewTarget->FindComponentByClass<UEnemiesCoordinatorComponent>();
		if (BTMemory->CoordinatorComponent.IsValid())
			CombatRole = BTMemory->CoordinatorComponent->RegisterEnemy(Pawn);
	}

	auto BlackboardMutable = OwnerComp->GetBlackboardComponent();
	BlackboardMutable->SetValueAsEnum(OutNpcCombatRoleTypeBBKey.SelectedKeyName, static_cast<uint8>(CombatRole));
	BTMemory->CombatLogicComponent->SetCombatRole(CombatRole);
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTService_UpdateCombatRole::TrySetCoordinatorRegistration(FBTMemory_UpdateCombatRole* BTMemory, APawn* BotCharacter,
                                                                UBlackboardComponent* BlackboardComponent, bool bRegister) const
{
	if (!BotCharacter) return;
	
	if (bRegister)
	{
		ENpcCombatRole NpcAttackRole = BTMemory->CoordinatorComponent->RegisterEnemy(BotCharacter);
		BTMemory->CombatLogicComponent->SetCombatRole(NpcAttackRole);
		BlackboardComponent->SetValueAsEnum(OutNpcCombatRoleTypeBBKey.SelectedKeyName, (uint8)NpcAttackRole);
	}
	else
	{
		BTMemory->CoordinatorComponent->UnregisterEnemy(BotCharacter);
		BlackboardComponent->ClearValue(OutNpcCombatRoleTypeBBKey.SelectedKeyName);
		BTMemory->CombatLogicComponent->SetCombatRole(ENpcCombatRole::None);
	}
}

void UBTService_UpdateCombatRole::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
		TargetActorBBKey.ResolveSelectedKey(*BB);
}

uint16 UBTService_UpdateCombatRole::GetInstanceMemorySize() const
{
	return sizeof(FBTMemory_UpdateCombatRole);
}

FString UBTService_UpdateCombatRole::GetStaticDescription() const
{
	return FString::Printf(TEXT("Target BB: %s\nCombat role BB: %s"),
		*TargetActorBBKey.SelectedKeyName.ToString(), *OutNpcCombatRoleTypeBBKey.SelectedKeyName.ToString());
}