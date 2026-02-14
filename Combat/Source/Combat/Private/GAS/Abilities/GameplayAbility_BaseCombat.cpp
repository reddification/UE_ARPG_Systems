// 


#include "GAS/Abilities/GameplayAbility_BaseCombat.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "GameplayAbilitiesDeveloperSettings.h"

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
#if !WITH_EDITOR
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
#else 
	// Don't set the actor info, CanActivate is called on the CDO

	// A valid AvatarActor is required. Simulated proxy check means only authority or autonomous proxies should be executing abilities.
	AActor* const AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (AvatarActor == nullptr || !ShouldActivateAbility(AvatarActor->GetLocalRole()))
	{
		return false;
	}

	//make into a reference for simplicity
	static FGameplayTagContainer DummyContainer;
	DummyContainer.Reset();

	// make sure the ability system component is valid, if not bail out.
	UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		return false;
	}

	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		ABILITY_LOG(Warning, TEXT("CanActivateAbility %s failed, called with invalid Handle"), *GetName());
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

		UE_LOG(LogAbilitySystem, Verbose, TEXT("%s: %s could not be activated due to GetUserAbilityActivationInhibited"), *GetNameSafe(ActorInfo->OwnerActor.Get()), *GetNameSafe(Spec->Ability));
		UE_VLOG(ActorInfo->OwnerActor.Get(), VLogAbilitySystem, Verbose, TEXT("%s could not be activated due to GetUserAbilityActivationInhibited"), *GetNameSafe(Spec->Ability));
		return false;
	}
	
	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();

	if (!AbilitySystemGlobals.ShouldIgnoreCooldowns() && !CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
	{
		UE_LOG(LogAbilitySystem, Verbose, TEXT("%s: %s could not be activated due to Cooldown (%s)"), *GetNameSafe(ActorInfo->OwnerActor.Get()), *GetNameSafe(Spec->Ability), OptionalRelevantTags ? *OptionalRelevantTags->ToStringSimple() : TEXT("Unknown"));
		UE_VLOG(ActorInfo->OwnerActor.Get(), VLogAbilitySystem, Verbose, TEXT("%s could not be activated due to Cooldown (%s)"), *GetNameSafe(Spec->Ability), OptionalRelevantTags ? *OptionalRelevantTags->ToStringSimple() : TEXT("Unknown"));
		return false;
	}

	if (!AbilitySystemGlobals.ShouldIgnoreCosts() && !CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		UE_LOG(LogAbilitySystem, Verbose, TEXT("%s: %s could not be activated due to Cost (%s)"), *GetNameSafe(ActorInfo->OwnerActor.Get()), *GetNameSafe(Spec->Ability), OptionalRelevantTags ? *OptionalRelevantTags->ToStringSimple() : TEXT("Unknown"));
		UE_VLOG(ActorInfo->OwnerActor.Get(), VLogAbilitySystem, Verbose, TEXT("%s could not be activated due to Cost (%s)"), *GetNameSafe(Spec->Ability), OptionalRelevantTags ? *OptionalRelevantTags->ToStringSimple() : TEXT("Unknown"));
		return false;
	}

	if (!DoesAbilitySatisfyTagRequirements(*AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags))
	{	
		// If the ability's tags are blocked, or if it has a "Blocking" tag or is missing a "Required" tag, then it can't activate.
		UE_LOG(LogAbilitySystem, Verbose, TEXT("%s: %s could not be activated due to Blocking Tags or Missing Required Tags (%s)"), *GetNameSafe(ActorInfo->OwnerActor.Get()), *GetNameSafe(Spec->Ability), OptionalRelevantTags ? *OptionalRelevantTags->ToStringSimple() : TEXT("Unknown"));
		UE_VLOG(ActorInfo->OwnerActor.Get(), VLogAbilitySystem, Verbose, TEXT("%s could not be activated due to Blocking Tags or Missing Required Tags (%s)"), *GetNameSafe(Spec->Ability), OptionalRelevantTags ? *OptionalRelevantTags->ToStringSimple() : TEXT("Unknown"));
		return false;
	}

	// Check if this ability's input binding is currently blocked
	if (AbilitySystemComponent->IsAbilityInputBlocked(Spec->InputID))
	{
		UE_LOG(LogAbilitySystem, Verbose, TEXT("%s: %s could not be activated due to blocked input ID %d"), *GetNameSafe(ActorInfo->OwnerActor.Get()), *GetNameSafe(Spec->Ability), Spec->InputID);
		UE_VLOG(ActorInfo->OwnerActor.Get(), VLogAbilitySystem, Verbose, TEXT("%s could not be activated due to blocked input ID %d"), *GetNameSafe(Spec->Ability), Spec->InputID);
		return false;
	}

	if (bHasBlueprintCanUse)
	{
		FGameplayTagContainer K2FailTags;
		if (K2_CanActivateAbility(*ActorInfo, Handle, K2FailTags) == false)
		{
			UE_LOG(LogAbilitySystem, Verbose, TEXT("%s: CanActivateAbility on %s failed, Blueprint override returned false"), *GetNameSafe(ActorInfo->OwnerActor.Get()), *GetNameSafe(Spec->Ability));
			UE_VLOG(ActorInfo->OwnerActor.Get(), VLogAbilitySystem, Verbose, TEXT("CanActivateAbility on %s failed, Blueprint override returned false"), *GetNameSafe(Spec->Ability));

			if (OptionalRelevantTags)
			{
				const FGameplayTag& FailTag = GetDefault<UGameplayAbilitiesDeveloperSettings>()->ActivateFailCanActivateAbilityTag;
				if (FailTag.IsValid())
				{
					OptionalRelevantTags->AddTag(FailTag);
				}

				OptionalRelevantTags->AppendTags(K2FailTags);
			}

			return false;
		}
	}

	return true;
#endif
}

// Same story as with the fucking method above
bool UGameplayAbility_BaseCombat::DoesAbilitySatisfyTagRequirements(
	const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
#if !WITH_EDITOR
	return Super::DoesAbilitySatisfyTagRequirements(AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags);
#else 
	// Define a common lambda to check for blocked tags
	bool bBlocked = false;
	auto CheckForBlocked = [&](const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB)
	{
		// Do we not have any tags in common?  Then we're not blocked
		if (ContainerA.IsEmpty() || ContainerB.IsEmpty() || !ContainerA.HasAny(ContainerB))
		{
			return;
		}

		if (OptionalRelevantTags)
		{
			// Ensure the global blocking tag is only added once
			if (!bBlocked)
			{
				UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
				const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
				OptionalRelevantTags->AddTag(BlockedTag);
			}

			// Now append all the blocking tags
			OptionalRelevantTags->AppendMatchingTags(ContainerA, ContainerB);
		}

		bBlocked = true;
	};

	// Define a common lambda to check for missing required tags
	bool bMissing = false;
	auto CheckForRequired = [&](const FGameplayTagContainer& TagsToCheck, const FGameplayTagContainer& RequiredTags)
	{
		// Do we have no requirements, or have met all requirements?  Then nothing's missing
		if (RequiredTags.IsEmpty() || TagsToCheck.HasAll(RequiredTags))
		{
			return;
		}

		if (OptionalRelevantTags)
		{
			// Ensure the global missing tag is only added once
			if (!bMissing)
			{
				UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
				const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;
				OptionalRelevantTags->AddTag(MissingTag);
			}

			FGameplayTagContainer MissingTags = RequiredTags; 
			MissingTags.RemoveTags(TagsToCheck.GetGameplayTagParents());
			OptionalRelevantTags->AppendTags(MissingTags);
		}

		bMissing = true;
	};

	// Start by checking all of the blocked tags first (so OptionalRelevantTags will contain blocked tags first)
	CheckForBlocked(GetAssetTags(), AbilitySystemComponent.GetBlockedAbilityTags());
	CheckForBlocked(AbilitySystemComponent.GetOwnedGameplayTags(), ActivationBlockedTags);
	if (SourceTags != nullptr)
	{
		CheckForBlocked(*SourceTags, SourceBlockedTags);
	}
	if (TargetTags != nullptr)
	{
		CheckForBlocked(*TargetTags, TargetBlockedTags);
	}

	// Now check all required tags
	CheckForRequired(AbilitySystemComponent.GetOwnedGameplayTags(), ActivationRequiredTags);
	if (SourceTags != nullptr)
	{
		CheckForRequired(*SourceTags, SourceRequiredTags);
	}
	if (TargetTags != nullptr)
	{
		CheckForRequired(*TargetTags, TargetRequiredTags);
	}

	if (!bBlocked && !bMissing)
	{
		// If it's a custom implementation that blocks, we can't specify exactly which tag so just use the generic
		bBlocked = AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags());
		if (bBlocked && OptionalRelevantTags)
		{
			UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
			const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
			OptionalRelevantTags->AddTag(BlockedTag);
		}
	}

	// We succeeded if there were no blocked tags and no missing required tags	
	return !bBlocked && !bMissing;
#endif
	
}
