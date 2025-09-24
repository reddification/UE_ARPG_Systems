// 


#include "GAS/Abilities/GameplayAbility_BaseCombat.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"

UGameplayAbility_BaseCombat::UGameplayAbility_BaseCombat()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

// This fucking bullshit GAS framework just won't debug straight. At some moment of development NPC melee attack ability activation was getting blocked for no clear reason
// and when debugging the fucking GameplayAbility.cpp the execution was just popping left right up and down and I was getting no fucking clue what the fuck was wrong
// so I freaked out and just copypasted the fucking code in the override in my own ability to debug it properly. This is why the code is 100% copypaste
bool UGameplayAbility_BaseCombat::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// A valid AvatarActor is required. Simulated proxy check means only authority or autonomous proxies should be executing abilities.
	AActor* const AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (AvatarActor == nullptr || !ShouldActivateAbility(AvatarActor->GetLocalRole()))
	{
		return false;
	}

	//make into a reference for simplicity
	static FGameplayTagContainer DummyContainer;
	DummyContainer.Reset();

	FGameplayTagContainer& OutTags = OptionalRelevantTags ? *OptionalRelevantTags : DummyContainer;

	// make sure the ability system component is valid, if not bail out.
	UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		return false;
	}

	if (AbilitySystemComponent->GetUserAbilityActivationInhibited())
	{
		/**
		 *	Input is inhibited (UI is pulled up, another ability may be blocking all other input, etc).
		 *	When we get into triggered abilities, we may need to better differentiate between CanActivate and CanUserActivate or something.
		 *	E.g., we would want LMB/RMB to be inhibited while the user is in the menu UI, but we wouldn't want to prevent a 'buff when I am low health'
		 *	ability to not trigger.
		 *	
		 *	Basically: CanActivateAbility is only used by user activated abilities now. If triggered abilities need to check costs/cooldowns, then we may
		 *	want to split this function up and change the calling API to distinguish between 'can I initiate an ability activation' and 'can this ability be activated'.
		 */ 
		return false;
	}
	
	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();

	if (!AbilitySystemGlobals.ShouldIgnoreCooldowns() && !CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
	{
		UE_VLOG(ActorInfo->OwnerActor.Get(), LogAbilitySystem, Verbose, TEXT("Ability could not be activated due to Cooldown: %s"), *GetName());
		return false;
	}

	if (!AbilitySystemGlobals.ShouldIgnoreCosts() && !CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		UE_VLOG(ActorInfo->OwnerActor.Get(), LogAbilitySystem, Verbose, TEXT("Ability could not be activated due to Cost: %s"), *GetName());
		return false;
	}

	if (!DoesAbilitySatisfyTagRequirements(*AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags))
	{	// If the ability's tags are blocked, or if it has a "Blocking" tag or is missing a "Required" tag, then it can't activate.
		UE_VLOG(ActorInfo->OwnerActor.Get(), LogAbilitySystem, Verbose, TEXT("Ability could not be activated due to Blocking Tags or Missing Required Tags: %s"), *GetName());
		return false;
	}

	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		UE_VLOG(ActorInfo->OwnerActor.Get(), LogAbilitySystem, Warning, TEXT("CanActivateAbility %s failed, called with invalid Handle"), *GetName());
		return false;
	}

	// Check if this ability's input binding is currently blocked
	if (AbilitySystemComponent->IsAbilityInputBlocked(Spec->InputID))
	{
		UE_VLOG(ActorInfo->OwnerActor.Get(), LogAbilitySystem, Verbose, TEXT("Ability could not be activated due to blocked input ID %i: %s"), Spec->InputID, *GetName());
		return false;
	}

	if (bHasBlueprintCanUse)
	{
		if (K2_CanActivateAbility(*ActorInfo, Handle, OutTags) == false)
		{
			UE_VLOG(ActorInfo->OwnerActor.Get(), LogAbilitySystem, Warning, TEXT("CanActivateAbility %s failed, blueprint refused"), *GetName());
			return false;
		}
	}

	return true;
}

// Same story as with the fucking method above
bool UGameplayAbility_BaseCombat::DoesAbilitySatisfyTagRequirements(
	const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// Check if any of this ability's tags are currently blocked
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}

	// Check to see the required/blocked tags for this ability
	if (ActivationBlockedTags.Num() || ActivationRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;
		AbilitySystemComponentTags.Reset();

		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(ActivationBlockedTags))
		{
			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(ActivationRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}
	
	return true;
}
