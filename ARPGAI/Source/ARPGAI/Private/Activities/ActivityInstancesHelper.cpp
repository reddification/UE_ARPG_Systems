#include "Activities/ActivityInstancesHelper.h"

#include "AIController.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcActivityComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

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

UNpcActivityComponent* GetNpcActivityComponent(const UBehaviorTreeComponent& OwnerComp)
{
	auto AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return nullptr;

	return AIController->FindComponentByClass<UNpcActivityComponent>();
}
