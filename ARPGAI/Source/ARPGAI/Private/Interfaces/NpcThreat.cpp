#include "Interfaces/NpcThreat.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcCombatLogicComponent.h"

TArray<AActor*> INpcThreat::GetCurrentEnemies_NpcThreat() const
{
	TArray<AActor*> Result;
	if (auto Pawn = Cast<APawn>(this))
	{
		if (auto CombatLogicComponent = GetNpcCombatLogicComponent(Pawn))
		{
			const auto& ActiveThreats = CombatLogicComponent->GetActiveThreats();
			Result.Reserve(ActiveThreats.Num());
			for (const auto& ActiveThreat : ActiveThreats)
				Result.Add(ActiveThreat.Key.Get());
		}
	}
	
	return Result;
}

AActor* INpcThreat::GetPrimaryCombatTarget_NpcThreat() const
{
	if (auto Pawn = Cast<APawn>(this))
	{
		if (auto CombatLogicComponent = GetNpcCombatLogicComponent(Pawn))
		{
			const auto& PrimaryCombatTargetData = CombatLogicComponent->GetPrimaryTargetData();
			return PrimaryCombatTargetData.IsValid() ? PrimaryCombatTargetData.Actor.Get() : nullptr;
		}
	}
	
	return nullptr;
}

bool INpcThreat::IsPrimaryTarget_NpcThreat(const AActor* Actor, const FGameplayTag& ForBehavior) const
{
	if (auto Pawn = Cast<APawn>(this))
	{
		if (auto CombatLogicComponent = GetNpcCombatLogicComponent(Pawn))
		{
			const auto& PrimaryCombatTargetData = CombatLogicComponent->GetPrimaryTargetData();
			return PrimaryCombatTargetData.BehaviorType == ForBehavior && PrimaryCombatTargetData.Actor == Actor;
		}
	}
	
	return false;
}
