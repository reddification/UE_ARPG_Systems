#include "GameplayBehaviors/GameplayBehavior_Interaction.h"

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
		}

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
	
	return false;
}

void UGameplayBehavior_Interaction::EndBehavior(AActor& Avatar, const bool bInterrupted)
{
	auto SmartObjectUserInteractionComponent = Avatar.FindComponentByClass<USmartObjectUserInteractionComponent>();
	if (ensure(SmartObjectUserInteractionComponent))
	{
		AActor* SmartObjectActor = SmartObjectUserInteractionComponent->GetActiveSmartObjectActor();
		if (ensure(SmartObjectActor))
			if (auto SmartObjectOwnerInteractionComponent = SmartObjectActor->FindComponentByClass<USmartObjectOwnerInteractionComponent>())
				SmartObjectOwnerInteractionComponent->OnInteractionEnded();
		
		SmartObjectUserInteractionComponent->StopUseSmartObject();
	}
	
	Super::EndBehavior(Avatar, bInterrupted);
}