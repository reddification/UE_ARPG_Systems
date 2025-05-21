


#include "BehaviorTree/Services/BTService_Surround.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/EnemiesCoordinatorComponent.h"

DEFINE_LOG_CATEGORY_STATIC(BTService_CoordinateBehaviorLog, Log, All);

UBTService_Surround::UBTService_Surround()
{
	NodeName = "Surround";
	SelectedMobSquadRoleBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_Surround, SelectedMobSquadRoleBBKey), StaticEnum<ENpcSquadRole>());
	bNotifyBecomeRelevant = 1;
}

void UBTService_Surround::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	const UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		SelectedMobSquadRoleBBKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTService_Surround::GetInstanceMemorySize() const
{
	return sizeof(FBTServiceSurroundMemory);
}

void UBTService_Surround::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	const APawn* CurrentTarget = Cast<APawn>(Blackboard->GetValueAsObject(ClosestTargetBBKey.SelectedKeyName));
	if (!ensure(CurrentTarget != nullptr))
	{
		SetNextTickTime(NodeMemory, FLT_MAX);
		return;
	}
	
	const UEnemiesCoordinatorComponent* Coordinator = CurrentTarget->FindComponentByClass<UEnemiesCoordinatorComponent>();
	if (!ensure(Coordinator != nullptr))
	{
		SetNextTickTime(NodeMemory, FLT_MAX);
		return;
	}

	FBTServiceSurroundMemory* SurroundMemory = reinterpret_cast<FBTServiceSurroundMemory*>(NodeMemory);
	SurroundMemory->Coordinator = Coordinator;		
}

void UBTService_Surround::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	FBTServiceSurroundMemory* SurroundMemory = reinterpret_cast<FBTServiceSurroundMemory*>(NodeMemory);
	auto MobCharacter = OwnerComp.GetAIOwner()->GetPawn();
	if (SurroundMemory->Coordinator->IsEnemyRegistered(MobCharacter))
	{
		ENpcSquadRole CurrentMobSquadRole = SurroundMemory->Coordinator->GetNpcSquadRole(MobCharacter);
		OwnerComp.GetBlackboardComponent()->SetValueAsEnum(SelectedMobSquadRoleBBKey.SelectedKeyName, (uint8)CurrentMobSquadRole);
	}
}
