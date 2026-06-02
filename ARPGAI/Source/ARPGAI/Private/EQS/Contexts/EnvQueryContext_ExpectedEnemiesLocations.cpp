#include "EQS/Contexts/EnvQueryContext_ExpectedEnemiesLocations.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcCombatLogicComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

void UEnvQueryContext_ExpectedEnemiesLocations::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                               FEnvQueryContextData& ContextData) const
{
	const APawn* Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(Pawn))
		return;

	UNpcCombatLogicComponent* CombatLogicComponent = GetNpcCombatLogicComponent(Pawn);
	if (!CombatLogicComponent)
		return;
	
	const auto& CombatKnownEnemies = CombatLogicComponent->GetCombatEnemiesMemory();
	TArray<FVector> ExpectedLocations;
	if (bAll)
	{
		ExpectedLocations.Reserve(CombatKnownEnemies.Num());
		for (const auto& KnownEnemy : CombatKnownEnemies)
			if (KnownEnemy.Value.bAlive)
				ExpectedLocations.Emplace(KnownEnemy.Value.LastSeenLocation);				
	}	
	else
	{
		// 8 Apr 2026 (aki): TODO sort maybe?
		ExpectedLocations.Reserve(TopNRecent);
		const double CurrentTime = GetWorld()->GetTimeSeconds();
		int AddedCount = 0;
		for (const auto& KnownEnemy : CombatKnownEnemies)
		{
			if (KnownEnemy.Value.bAlive && CurrentTime - KnownEnemy.Value.LastUpdateTime <= RelevancyTimeThreshold)
			{
				ExpectedLocations.Emplace(KnownEnemy.Value.LastSeenLocation);
				AddedCount++;
				if (AddedCount >= TopNRecent)
					break;
			}
		}
	}
	
	UEnvQueryItemType_Point::SetContextHelper(ContextData, ExpectedLocations);
}
