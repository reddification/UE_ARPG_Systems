// 


#include "GAS/Abilities/GameplayAbility_NpcHitReact.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/CombatGameplayTags.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/NpcCombatant.h"

void UGameplayAbility_NpcHitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive() || bIsAbilityEnding)
		return;
	
	auto Combatant = Cast<ICombatant>(ActorInfo->OwnerActor.Get());;
	auto NpcCombatant = Cast<INpcCombatant>(ActorInfo->OwnerActor.Get());
	
	float Reaction = NpcCombatant->GetReaction();
	float NormalizedStamina = Combatant->GetStaminaRatio();
	if (FMath::RandRange(0.f, 1.f) <= 2.f * Reaction * NormalizedStamina)
	{
		FGameplayEventData EventData;
		ActorInfo->AbilitySystemComponent->HandleGameplayEvent(CombatGameplayTags::Combat_Ability_Backdash_Event_Activate, &EventData);
	}
}