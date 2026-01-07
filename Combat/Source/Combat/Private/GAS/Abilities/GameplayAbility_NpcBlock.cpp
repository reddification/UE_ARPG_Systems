// 


#include "GAS/Abilities/GameplayAbility_NpcBlock.h"

#include "Components/NpcBlockComponent.h"
#include "GAS/Data/GameplayAbilityTargetData_BlockAttack.h"
#include "Helpers/GASHelpers.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/NpcCombatant.h"

void UGameplayAbility_NpcBlock::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	float BackstepChance = 0.5f;
	auto AvatarActor = ActorInfo->AvatarActor.Get();
	auto NpcCombatant = Cast<INpcCombatant>(AvatarActor);
	auto Combatant = Cast<ICombatant>(AvatarActor);
	if (NpcCombatant)
	{
		float Reaction = NpcCombatant->GetReaction();
		float Intelligence = NpcCombatant->GetIntelligence();
		BackstepChance = (Reaction + Intelligence) * Combatant->GetStaminaRatio(); // why such formula - because i just felt that way
	}

	if (FMath::RandRange(0.f, 1.f) < BackstepChance)
	{
		FGameplayTagContainer OwnerTags;
		auto OwnerTagInterface = Cast<IGameplayTagAssetInterface>(ActorInfo->AvatarActor.Get());
		OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
		UAnimMontage* BackstepMontage = nullptr; 
		for (const auto& BackstepMontageOption : BackstepMontages)
		{
			if (BackstepMontageOption.ContextTags.IsEmpty() || BackstepMontageOption.ContextTags.Matches(OwnerTags))
			{
				BackstepMontage = BackstepMontageOption.Montages_Deprecated[FMath::RandRange(0, BackstepMontageOption.Montages_Deprecated.Num() - 1)];
				break;				
			}
		}

		if (BackstepMontage && ensure(ActorInfo->GetAnimInstance()))
		{
			ActorInfo->GetAnimInstance()->Montage_Play(BackstepMontage);
		}
	}
}

bool UGameplayAbility_NpcBlock::StartBlocking(const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	auto BlockComponent = ActorInfo->AvatarActor->FindComponentByClass<UNpcBlockComponent>();
	if (!ensure(BlockComponent))
		return false;

	const FGameplayAbilityTargetData_BlockAttack* BlockActivationData = GetActivationData<FGameplayAbilityTargetData_BlockAttack>(TriggerEventData->TargetData);
	if (!ensure(BlockActivationData))
		return false;
	
	BlockComponent->StartBlocking(BlockActivationData->Attacker, BlockActivationData->IncomingAttackTrajectory);
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
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		auto NpcCombatant = Cast<INpcCombatant>(ActorInfo->AvatarActor.Get());
		if (ensure(NpcCombatant))
			NpcCombatant->BlockCompleted();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_NpcBlock::OnAttackParried()
{
	Super::OnAttackParried();
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
