#include "GAS/Abilities/GameplayAbility_NpcHitReact.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/CombatGameplayTags.h"
#include "Data/CombatLogChannels.h"
#include "GameFramework/Character.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/NpcCombatant.h"

void UGameplayAbility_NpcHitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive() || bIsAbilityEnding)
		return;
	
	GetWorld()->GetTimerManager().SetTimer(AutoCorrectionTimer, this, &UGameplayAbility_NpcHitReact::AutoCorrectEndAbility, 4.f, false);
}

void UGameplayAbility_NpcHitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (GetWorld())
		GetWorld()->GetTimerManager().ClearTimer(AutoCorrectionTimer);
}

void UGameplayAbility_NpcHitReact::AutoCorrectEndAbility()
{
	if (!IsActive())
		return;
	
	if (CurrentActorInfo->AvatarActor.IsValid())
	{
		UE_VLOG_UELOG(CurrentActorInfo->AvatarActor.Get(), LogCombat_HitReact, Error, TEXT("FUCK! NPC's HitReact ability didn't finish on its own. Had to finish manually"));
	}
	
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, GetCurrentActivationInfo(), true, false);
}
