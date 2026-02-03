// 


#include "GAS/Executions/AttackRangeMMC.h"

#include "Data/MeleeCombatSettings.h"
#include "Helpers/CombatCommonHelpers.h"
#include "Interfaces/ICombatant.h"

float UAttackRangeMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	auto InstigatorActor = Spec.GetEffectContext().GetInstigator();
	auto Combatant = Cast<ICombatant>(InstigatorActor);
	if (Combatant == nullptr)
		return 0.f;
	
	auto DamageCollisionComponents = Combatant->GetDamageCollisionsComponents();
	if (DamageCollisionComponents.Num() == 0)
		return 0.f;

	float MaxRange = 0.f;
	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	auto CombatCollisionShapes = GetCombatCollisionShapes(CombatSettings->CombatCollisionName, DamageCollisionComponents, InstigatorActor);
	
	for (int i = 0; i < CombatCollisionShapes.Num(); i++)
		if (CombatCollisionShapes[i].GetRange() > MaxRange)
			MaxRange = CombatCollisionShapes[i].GetRange();

	//arm length + leaning body would add approximately this value to overall weapon reach (also subtracting grip length)
	float constexpr AverageReach = 70.f; 
	return MaxRange + AverageReach;
}
