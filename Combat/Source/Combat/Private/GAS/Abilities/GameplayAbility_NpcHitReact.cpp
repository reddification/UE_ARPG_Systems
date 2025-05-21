// 


#include "GAS/Abilities/GameplayAbility_NpcHitReact.h"

#include "Interfaces/ICombatant.h"
#include "Interfaces/NpcCombatant.h"

void UGameplayAbility_NpcHitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	auto Combatant = Cast<ICombatant>(ActorInfo->OwnerActor.Get());;
	auto NpcCombatant = Cast<INpcCombatant>(ActorInfo->OwnerActor.Get());
	
	float Reaction = NpcCombatant->GetReaction();
	float NormalizedStamina = Combatant->GetStaminaRatio();
	if (FMath::RandRange(0.f, 1.f) < 2.f * Reaction * NormalizedStamina)
	// if (true)
	{
		UAnimMontage* BackstepMontage = nullptr;
		FGameplayTagContainer OwnerTags;
		auto OwnerTagInterface = Cast<IGameplayTagAssetInterface>(ActorInfo->AvatarActor.Get());
		OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
		for (const auto& BackstepMontageOption : BackstepMontageOptions)
		{
			if (BackstepMontageOption.ContextTags.Matches(OwnerTags))
			{
				BackstepMontage = BackstepMontageOption.Montages[FMath::RandRange(0, BackstepMontageOption.Montages.Num() - 1)];
				break;				
			}
		}

		if (BackstepMontage)
			ActorInfo->GetAnimInstance()->Montage_Play(BackstepMontage);
	}
}
