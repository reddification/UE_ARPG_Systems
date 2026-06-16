#include "BehaviorTree/Decorators/BTDecorator_MustSearchCombatTarget.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcCombatLogicComponent.h"

UBTDecorator_MustSearchCombatTarget::UBTDecorator_MustSearchCombatTarget()
{
	NodeName = "Must search enemy";
}

bool UBTDecorator_MustSearchCombatTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	auto NpcCombatLogicComponent = GetNpcCombatLogicComponent(OwnerComp);
	const auto& EnemiesMemory = NpcCombatLogicComponent->GetCombatEnemiesMemory();
	auto AreasComponent = GetNpcAreasComponent(OwnerComp);
	bool bTerritorial = AreasComponent ? AreasComponent->HasAreas() : false;
	FVector OwnerLocation = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
	for (const auto& EnemyMemory : EnemiesMemory)
	{
		if (!EnemyMemory.Value.bAlive)
			continue;
		
		if (bTerritorial)
		{
			if (!AreasComponent->IsLocationWithinNpcArea(EnemyMemory.Value.LastSeenLocation, TerritoryExtent))
				continue;
		}
		else
		{
			if ((OwnerLocation - EnemyMemory.Value.LastSeenLocation).SizeSquared() > DistanceThreshold * DistanceThreshold)
				continue;
		}
		
		return true;
	}
	
	return false;
}

FString UBTDecorator_MustSearchCombatTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Distance threshold = %.2f\nTerritory extent = %.2f"), DistanceThreshold, TerritoryExtent);
}
