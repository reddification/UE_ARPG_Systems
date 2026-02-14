#include "GAS/Abilities/GameplayAbility_NpcBlock.h"

#include "AbilitySystemComponent.h"
#include "Components/NpcBlockComponent.h"
#include "GAS/Data/GameplayAbilityTargetData_Block.h"
#include "GAS/Data/GameplayAbilityTargetData_BlockIncomingAttack.h"
#include "Helpers/GASHelpers.h"
#include "Interfaces/NpcCombatant.h"

bool UGameplayAbility_NpcBlock::StartBlocking(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData)
{
	auto BlockComponent = ActorInfo->AvatarActor->FindComponentByClass<UNpcBlockComponent>();
	if (!ensure(BlockComponent))
		return false;

	if (const auto* BlockIncomingAttackActivationData = GetActivationData<FGameplayAbilityTargetData_BlockIncomingAttack>(TriggerEventData->TargetData))
		BlockComponent->StartBlocking(BlockIncomingAttackActivationData->Attacker, BlockIncomingAttackActivationData->IncomingAttackTrajectory);
	else if (const auto* BlockAngleActivationData = GetActivationData<FGameplayAbilityTargetData_Block>(TriggerEventData->TargetData))
		BlockComponent->StartBlocking(BlockAngleActivationData->BlockAngle);
	else
		return ensure(false);
	
	return true;	
}

void UGameplayAbility_NpcBlock::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	auto BlockComponent = ActorInfo->AvatarActor->FindComponentByClass<UNpcBlockComponent>();
	if (!ensure(BlockComponent))
		return;

	BlockComponent->OnNpcFinishedBlockingEvent.BindUObject(this, &UGameplayAbility_NpcBlock::OnNpcFinishedBlocking);
}

void UGameplayAbility_NpcBlock::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		auto NpcCombatant = Cast<INpcCombatant>(ActorInfo->AvatarActor.Get());
		if (ensure(NpcCombatant))
			NpcCombatant->BlockCompleted(!bWasCancelled);
	}
}

void UGameplayAbility_NpcBlock::OnNpcFinishedBlocking()
{
	if (!IsActive() || bIsAbilityEnding)
		return;
	
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
