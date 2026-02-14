// 


#include "GAS/Abilities/GameplayAbility_PhysicalImpact.h"

#include "Data/CombatGameplayTags.h"

UGameplayAbility_PhysicalImpact::UGameplayAbility_PhysicalImpact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_PhysicalImpact_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}
