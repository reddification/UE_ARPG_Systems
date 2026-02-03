// 


#include "BehaviorTree/Decorators/BTDecorator_IsAllyInCombatWithTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/EnemiesCoordinatorComponent.h"
#include "Subsystems/NpcSquadSubsystem.h"

UBTDecorator_IsAllyInCombatWithTarget::UBTDecorator_IsAllyInCombatWithTarget()
{
	NodeName = "Are allies in combat with target";
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsAllyInCombatWithTarget, TargetBBKey), AActor::StaticClass());
	bTickIntervals = true;
	bNotifyTick = true;
}

bool UBTDecorator_IsAllyInCombatWithTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	auto Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetBBKey.SelectedKeyName));
	if (Target == nullptr)
		return false;
	
	auto EnemiesCoordinatorComponent = Target->FindComponentByClass<UEnemiesCoordinatorComponent>();
	if (EnemiesCoordinatorComponent == nullptr)
		return false;
	
	auto NpcPawn = OwnerComp.GetAIOwner()->GetPawn();
	auto NpcSquadSubsystem = UNpcSquadSubsystem::Get(NpcPawn);
	auto Allies = NpcSquadSubsystem->GetAllies(NpcPawn, false, true);
	if (Allies.Num() < RequiredAlliesCount)
		return false;
	
	int TotalAlliesCountInCombat = 0;
	for (const auto& Ally : Allies)
	{
		auto Role = EnemiesCoordinatorComponent->GetEnemyRole(Ally);
		if (Role == ENpcCombatRole::Attacker || Role == ENpcCombatRole::Surrounder)
			TotalAlliesCountInCombat++;
		
		if (TotalAlliesCountInCombat >= RequiredAlliesCount)
			break;
	}
	
	return TotalAlliesCountInCombat >= RequiredAlliesCount;
}

void UBTDecorator_IsAllyInCombatWithTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	if (FlowAbortMode == EBTFlowAbortMode::None)
	{
		SetNextTickTime(NodeMemory, FLT_MAX);
		return;
	}
	
	ConditionalFlowAbort(OwnerComp, EBTDecoratorAbortRequest::ConditionResultChanged);
	SetNextTickTime(NodeMemory, TickInterval);
}

FString UBTDecorator_IsAllyInCombatWithTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Check if at least %d allies in combat with %s\nUpdate each %.2f s\n%s"), RequiredAlliesCount, 
		*TargetBBKey.SelectedKeyName.ToString(), TickInterval, *Super::GetStaticDescription());
}
