// 


#include "GAS/Abilities/GameplayAbility_ReceiveGuardBreak.h"

#include "Data/CombatGameplayTags.h"

UGameplayAbility_ReceiveGuardBreak::UGameplayAbility_ReceiveGuardBreak()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_ReceiveGuardBreak_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}
