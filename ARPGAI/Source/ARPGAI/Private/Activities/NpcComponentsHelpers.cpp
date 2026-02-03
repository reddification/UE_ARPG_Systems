#include "Activities/NpcComponentsHelpers.h"

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

UNpcBehaviorEvaluatorComponent* GetNpcBehaviorEvaluatorComponent(const UBehaviorTreeComponent& OwnerComp)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		return AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();

	return nullptr;
}
