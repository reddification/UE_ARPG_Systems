// 


#include "GAS/Abilities/GameplayAbility_NpcBlock.h"

#include "AbilitySystemComponent.h"
#include "Components/NpcBlockComponent.h"
#include "GAS/Data/GameplayAbilityTargetData_Block.h"
#include "GAS/Data/GameplayAbilityTargetData_BlockIncomingAttack.h"
#include "Helpers/GASHelpers.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/NpcCombatant.h"

void UGameplayAbility_NpcBlock::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive() || bIsAbilityEnding)
		return;
	
	float BackdashChance = 0.5f;
	auto AvatarActor = ActorInfo->AvatarActor.Get();
	auto NpcCombatant = Cast<INpcCombatant>(AvatarActor);
	auto Combatant = Cast<ICombatant>(AvatarActor);
	if (NpcCombatant)
	{
		float Reaction = NpcCombatant->GetReaction();
		float Intelligence = NpcCombatant->GetIntelligence();
		BackdashChance = (Reaction + Intelligence) * Combatant->GetStaminaRatio(); // why such formula - because i just felt that way
	}
	
	if (FMath::RandRange(0.f, 1.f) <= BackdashChance)
	{
		FGameplayEventData Payload;
		ActorInfo->AbilitySystemComponent->HandleGameplayEvent(CombatGameplayTags::Combat_Ability_Backdash_Event_Activate, &Payload);
	}
}

bool UGameplayAbility_NpcBlock::StartBlocking(const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayEventData* TriggerEventData)
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
			NpcCombatant->BlockCompleted();
	}
}

void UGameplayAbility_NpcBlock::OnAttackParried(AActor* Attacker)
{
	Super::OnAttackParried(Attacker);
	auto NpcCombatant = Cast<INpcCombatant>(GetCurrentActorInfo()->AvatarActor.Get());
	if (ensure(NpcCombatant))
	{
		NpcCombatant->ReportSuccessfulParry();
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	}
}

void UGameplayAbility_NpcBlock::OnNpcFinishedBlocking()
{
	if (!IsActive())
		return;
	
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
