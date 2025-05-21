#pragma once

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/NpcComponent.h"

inline UNpcComponent* GetMobComponent(const UBehaviorTreeComponent& BehaviorTreeComponent)
{
	if (AAIController* AIController = BehaviorTreeComponent.GetAIOwner())
	{
		if (APawn* Pawn = AIController->GetPawn())
		{
			return Pawn->FindComponentByClass<UNpcComponent>();
		}
	}

	return nullptr;
}
