// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_CombatBase.h"

#include "AIController.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Data/CombatLogChannels.h"

UGameplayAbility_CombatBase::UGameplayAbility_CombatBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGameplayAbility_CombatBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	// if at any moment player will be able to possess/unpossess an NPC, then this logic must be moved to ActivateAbility 
}

void UGameplayAbility_CombatBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bWasCancelledByAI = false;
	if (auto Pawn = Cast<APawn>(ActorInfo->OwnerActor))
	{
		if (Pawn->Controller && Pawn->Controller->IsA<AAIController>())
		{
			WaitAIAbortGameplayEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AbortTag);
			WaitAIAbortGameplayEvent->EventReceived.AddDynamic(this, &UGameplayAbility_CombatBase::OnAIAbort);
			WaitAIAbortGameplayEvent->ReadyForActivation();
			bAIOwner = true;
		}
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_CombatBase::ActivateAbility [%s]"), *GetClass()->GetName());
}

void UGameplayAbility_CombatBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                             bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (WaitAIAbortGameplayEvent)
	{
		WaitAIAbortGameplayEvent->EventReceived.RemoveAll(this);
		WaitAIAbortGameplayEvent = nullptr;
	}

	UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_CombatBase::EndAbility [%s]"), *GetClass()->GetName());
	
	// if (!bWasCancelledByAI && bAIOwner)
	// {
	// 	SendMessageToAIController(ActorInfo->AvatarActor.Get(), bWasCancelled ? AIMessageAbilityCanceledTag : AIMessageAbilityCompletedTag, !bWasCancelled);
	// }
}

void UGameplayAbility_CombatBase::OnAIAbort(FGameplayEventData Payload)
{
	if (bIsActive)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
		bWasCancelledByAI = true;
	}
}