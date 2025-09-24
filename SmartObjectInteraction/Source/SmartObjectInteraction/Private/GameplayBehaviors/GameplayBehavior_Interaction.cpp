#include "GameplayBehaviors/GameplayBehavior_Interaction.h"

#include "GameplayBehaviorSmartObjectBehaviorDefinition.h"
#include "SmartObjectInteractionLogChannels.h"
#include "SmartObjectSubsystem.h"
#include "Components/SmartObjectOwnerInteractionComponent.h"
#include "Components/SmartObjectUserInteractionComponent.h"
#include "GameplayBehaviors/GameplayBehaviorConfig_Interaction.h"

UGameplayBehavior_Interaction::UGameplayBehavior_Interaction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// not sure if its reliable to have it not instantiated
	InstantiationPolicy = EGameplayBehaviorInstantiationPolicy::DontInstantiate;
	// InstantiationPolicy = EGameplayBehaviorInstantiationPolicy::Instantiate;
}

bool UGameplayBehavior_Interaction::Trigger(AActor& Avatar, const UGameplayBehaviorConfig* Config, AActor* SmartObjectOwner)
{
	// Super::Trigger(Avatar, Config, SmartObjectOwner); // i don't remember why is Super::Trigger not called... maybe it was deliberately
	UE_VLOG(&Avatar, LogSmartObjectInteraction, Verbose, TEXT("UGameplayBehavior_Interaction::Trigger"));
	const UGameplayBehaviorConfig_Interaction* InteractionConfig = Cast<const UGameplayBehaviorConfig_Interaction>(Config);
	auto SmartObjectUserInteractionComponent = Avatar.FindComponentByClass<USmartObjectUserInteractionComponent>();
	if (ensure(SmartObjectUserInteractionComponent))
	{
		if (auto SmartObjectOwnerInteractionComponent = SmartObjectOwner->FindComponentByClass<USmartObjectOwnerInteractionComponent>())
		{
			FGameplayTagContainer GrantedSmartObjectOwnerActorTags;
			if (InteractionConfig->bSmartObjectActorChooseSingleRandomGrantedTag && !InteractionConfig->GrantTagsToSmartObjectActorOnStartUsing.IsEmpty())
			{
				const TArray<FGameplayTag>& GrantedSOTags = InteractionConfig->GrantTagsToSmartObjectActorOnStartUsing.GetGameplayTagArray();
				GrantedSmartObjectOwnerActorTags = (GrantedSOTags.Num() > 1
					? GrantedSOTags[FMath::RandRange(0, GrantedSOTags.Num() - 1)]
					: GrantedSOTags[0]).GetSingleTagContainer();
			}
			else
			{
				GrantedSmartObjectOwnerActorTags = InteractionConfig->GrantTagsToSmartObjectActorOnStartUsing;
			}
			
			SmartObjectOwnerInteractionComponent->OnInteractionStarted(GrantedSmartObjectOwnerActorTags, InteractionConfig->bSmartObjectActorTagsPermanent);
			for (const auto& InteractionFunction : InteractionConfig->InteractionFunctions)
				if (auto InteractionFunctionPtr = InteractionFunction.GetPtr())
					InteractionFunctionPtr->OnStart(&Avatar, SmartObjectOwner);
		}

		// (aki) 01.09.2025: TODO move to FGameplayInteractionFunctionBase child
		FGameplayTagContainer GrantedSmartObjectUserTags;
		if (InteractionConfig->bSmartObjectUserChooseSingleRandomGrantedTag && !InteractionConfig->GrantTagsToUserOnStartUsing.IsEmpty())
		{
			const TArray<FGameplayTag>& GrantedUserTags = InteractionConfig->GrantTagsToUserOnStartUsing.GetGameplayTagArray();
			GrantedSmartObjectUserTags = (GrantedUserTags.Num() > 1
				? GrantedUserTags[FMath::RandRange(0, GrantedUserTags.Num() - 1)]
				: GrantedUserTags[0]).GetSingleTagContainer();
		}
		else
		{
			GrantedSmartObjectUserTags = InteractionConfig->GrantTagsToUserOnStartUsing;
		}
		
		return SmartObjectUserInteractionComponent->StartUseSmartObject(InteractionConfig->GestureTag, SmartObjectOwner, GrantedSmartObjectUserTags,
			InteractionConfig->bSmartObjectUserTagsPermanent);
	}

	UE_VLOG(&Avatar, LogSmartObjectInteraction, Warning, TEXT("UGameplayBehavior_Interaction::Trigger: failure"));
	return false;
}

void UGameplayBehavior_Interaction::EndBehavior(AActor& Avatar, const bool bInterrupted)
{
	UE_VLOG(&Avatar, LogSmartObjectInteraction, Verbose, TEXT("UGameplayBehavior_Interaction::EndBehavior"));
	auto SmartObjectUserInteractionComponent = Avatar.FindComponentByClass<USmartObjectUserInteractionComponent>();
	if (!ensure(SmartObjectUserInteractionComponent))
	{
		Super::EndBehavior(Avatar, bInterrupted);
		return;
	}
	
	AActor* SmartObjectActor = SmartObjectUserInteractionComponent->GetActiveSmartObjectActor();
	if (SmartObjectActor == nullptr)
	{
		UE_VLOG(&Avatar, LogSmartObjectInteraction, Warning, TEXT("UGameplayBehavior_Interaction::EndBehavior: no active smart object actor"));
		ensure(false);
	}
	
	if (SmartObjectActor)
		if (auto SmartObjectOwnerInteractionComponent = SmartObjectActor->FindComponentByClass<USmartObjectOwnerInteractionComponent>())
			SmartObjectOwnerInteractionComponent->OnInteractionEnded();

	// caching ClaimedHandle before calling ->StopUseSmartObject as it can reset the claim 
	FSmartObjectClaimHandle ClaimedHandle = ClaimedHandle = SmartObjectUserInteractionComponent->GetActiveSOCH();
	SmartObjectUserInteractionComponent->StopUseSmartObject();

	if (SmartObjectActor != nullptr)
	{
		auto SmartObjectSubsystem = USmartObjectSubsystem::GetCurrent(SmartObjectActor->GetWorld());
		if (IsValid(SmartObjectSubsystem))
		{
			const auto* SmartObjectGameplayBehaviorDefinition = SmartObjectSubsystem->GetBehaviorDefinition<UGameplayBehaviorSmartObjectBehaviorDefinition>(ClaimedHandle);
			const auto InteractionConfig = SmartObjectGameplayBehaviorDefinition != nullptr
				? Cast<UGameplayBehaviorConfig_Interaction>(SmartObjectGameplayBehaviorDefinition->GameplayBehaviorConfig)
				: nullptr;
			if (InteractionConfig)
			{
				for (const auto& InteractionFunction : InteractionConfig->InteractionFunctions)
					if (auto InteractionFunctionPtr = InteractionFunction.GetPtr())
						InteractionFunctionPtr->OnEnd(&Avatar, SmartObjectActor);
			}
			else
			{
				UE_VLOG(&Avatar, LogSmartObjectInteraction, Warning, TEXT("UGameplayBehavior_Interaction::EndBehavior: interaction config is nullptr"));
			}
		}
	}

	Super::EndBehavior(Avatar, bInterrupted);
}