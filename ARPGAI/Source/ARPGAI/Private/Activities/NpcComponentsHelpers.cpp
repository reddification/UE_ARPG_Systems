#include "Activities/NpcComponentsHelpers.h"

#include "AIController.h"
#include "Components/NpcComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcHealingComponent.h"
#include "Components/RoleplayComponent.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent2.h"
#include "Components/Controller/NpcConversationComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Components/Controller/NpcMemoryComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Interfaces/NpcGoalManager.h"

UNpcComponent* GetNpcComponent(const UBehaviorTreeComponent& OwnerComp)
{
	auto AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return nullptr;

	auto Pawn = AIController->GetPawn();
	if (!Pawn)
		return nullptr;
	
	return Pawn->FindComponentByClass<UNpcComponent>();
}

UNpcComponent* GetNpcComponent(const APawn* Pawn)
{
	return IsValid(Pawn) ? Pawn->FindComponentByClass<UNpcComponent>() : nullptr;
}

INpcGoalManager* GetNpcGoalManager(const UBehaviorTreeComponent& OwnerComp)
{
	auto AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return nullptr;

	return Cast<INpcGoalManager>( AIController);
}

UNpcFlowComponent* GetNpcFlowComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		return AIController->FindComponentByClass<UNpcFlowComponent>();

	return nullptr;
}

UNpcFlowComponent* GetNpcFlowComponent(const APawn* Pawn)
{
	if (auto Controller = Pawn->GetController())
		return Controller->FindComponentByClass<UNpcFlowComponent>();
	
	return nullptr;
}

UNpcBehaviorEvaluatorComponent* GetNpcBehaviorEvaluatorComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		return AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();

	return nullptr;
}

UNpcAttitudesComponent* GetNpcAttitudesComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		return GetNpcAttitudesComponent(AIController);
	
	return nullptr;
}

UNpcAttitudesComponent* GetNpcAttitudesComponent(const AAIController* AIController)
{
	if (auto Pawn = AIController->GetPawn())
		return GetNpcAttitudesComponent(Pawn);

	return nullptr;
}

UNpcAttitudesComponent* GetNpcAttitudesComponent(const APawn* Pawn)
{
	return Pawn->FindComponentByClass<UNpcAttitudesComponent>();
}

URoleplayComponent* GetRoleplayComponent(const AAIController* AIController)
{
	return AIController->GetPawn()->FindComponentByClass<URoleplayComponent>();
}

UNpcAreasComponent* GetNpcAreasComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())
			return GetNpcAreasComponent(Pawn);
	
	return nullptr;
}

UNpcAreasComponent* GetNpcAreasComponent(APawn* Pawn)
{
	return Pawn->FindComponentByClass<UNpcAreasComponent>();
}

UNpcHealingComponent* GetNpcHealComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())	
			return Pawn->FindComponentByClass<UNpcHealingComponent>();

	return nullptr;
}

UNpcPerceptionComponent* GetNpcShortTermMemoryComponent(const UBehaviorTreeComponent& OwnerComp)
{
	return Cast<UNpcPerceptionComponent>(OwnerComp.GetAIOwner()->GetAIPerceptionComponent());
}

UNpcPerceptionComponent* GetNpcShortTermMemoryComponent(const APawn* OwnerPawn)
{
	return OwnerPawn->GetController()->FindComponentByClass<UNpcPerceptionComponent>();
}

UNpcMemoryComponent* GetNpcLongTermMemoryComponent(const UBehaviorTreeComponent& OwnerComp)
{
	return OwnerComp.GetAIOwner()->FindComponentByClass<UNpcMemoryComponent>();
}

UNpcMemoryComponent* GetNpcLongTermMemoryComponent(const APawn* OwnerPawn)
{
	return OwnerPawn->GetController()->FindComponentByClass<UNpcMemoryComponent>();
}

UNpcConversationComponent* GetNpcConversationComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())
			return Pawn->FindComponentByClass<UNpcConversationComponent>();

	return nullptr;
}

UNpcConversationComponent* GetNpcConversationComponent(const APawn* Pawn)
{
	return Pawn->FindComponentByClass<UNpcConversationComponent>();
}

UNpcCombatLogicComponent* GetNpcCombatLogicComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())	
			return Pawn->FindComponentByClass<UNpcCombatLogicComponent>();

	return nullptr;
}

UNpcCombatLogicComponent* GetNpcCombatLogicComponent(const APawn* Pawn)
{
	return IsValid(Pawn) ? Pawn->FindComponentByClass<UNpcCombatLogicComponent>() : nullptr;	
}

UNpcBehaviorEvaluatorComponent2* GetNpcBehaviorEvaluatorComponent_v2(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		return AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent2>();
	
	return nullptr;
}
