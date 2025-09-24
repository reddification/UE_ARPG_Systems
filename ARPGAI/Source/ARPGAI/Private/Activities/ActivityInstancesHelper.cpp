#include "Activities/ActivityInstancesHelper.h"

#include "AIController.h"
#include "Components/NpcComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
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

class UNpcAttitudesComponent* GetNpcAttitudesComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())
			return Pawn->FindComponentByClass<UNpcAttitudesComponent>();

	return nullptr;
}

class UNpcCombatLogicComponent* GetNpcCombatLogicComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())	
			return Pawn->FindComponentByClass<UNpcCombatLogicComponent>();

	return nullptr;
}

class UNpcBehaviorEvaluatorComponent* GetNpcBehaviorEvaluatorComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		return AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();

	return nullptr;
}
