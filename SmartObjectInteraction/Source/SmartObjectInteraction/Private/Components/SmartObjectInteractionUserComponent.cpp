#include "SmartObjectInteractionLogChannels.h"
#include "SmartObjectSubsystem.h"
#include "Components/SmartObjectUserInteractionComponent.h"
#include "Interfaces/SmartObjectOwnerInteractionInterface.h"
#include "Interfaces/SmartObjectUserInteractionInterface.h"

USmartObjectUserInteractionComponent::USmartObjectUserInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool USmartObjectUserInteractionComponent::StartUseSmartObject(const FGameplayTag& GestureTag, AActor* SmartObjectOwner,
	const FGameplayTagContainer& InGrantedTags, bool bSmartObjectUserTagsPermanent_In)
{
	auto SmartObjectUserInterface = Cast<ISmartObjectUserInteractionInterface>(GetOwner());
	if (!ensure(SmartObjectUserInterface))
		return false;

	// 19.11.2024 @AK: Currently allowing GestureTag to be empty, because there are some cases where smart object is only needed to apply
	// posture (gait) tags on a character, for example guard spots for guardians - they don't do gesture, they just apply a random posture
	bool bStarted = GestureTag.IsValid() ? SmartObjectUserInterface->StartUsingSmartObject(GestureTag) : SmartObjectUserInterface->StartUsingSmartObject();
	if (!ensure(bStarted))
		return false;
	
	ActiveSmartObjectOwner = SmartObjectOwner;
	if (!InGrantedTags.IsEmpty())
	{
		GrantedTags = InGrantedTags;
		bSmartObjectGrantedUserTagsPermanent = bSmartObjectUserTagsPermanent_In;
		SmartObjectUserInterface->GrantSmartObjectUsageTags(InGrantedTags);
	}

	FGameplayTagContainer InteractionTags;
	auto SmartObjectSubsystem = USmartObjectSubsystem::GetCurrent(GetWorld());
	SmartObjectSubsystem->GetSlotView(ActiveSOCH.SlotHandle).GetActivityTags(InteractionTags);

	SmartObjectUserInterface->GrantSmartObjectUsageTags(InteractionTags);

	return true;
}

void USmartObjectUserInteractionComponent::StopUseSmartObject()
{
	if (!ActiveSmartObjectOwner.IsValid())
		return;
	
	auto SmartObjectUserInterface = Cast<ISmartObjectUserInteractionInterface>(GetOwner());
	if (SmartObjectUserInterface)
		SmartObjectUserInterface->StopUsingSmartObject();

	if (!GrantedTags.IsEmpty() && !bSmartObjectGrantedUserTagsPermanent)
	{
		SmartObjectUserInterface->RemoveSmartObjectInteractionTags(GrantedTags);
		GrantedTags.Reset();
	}

	bool bInteractionTagsRemoved = false;
	FGameplayTagContainer InteractionTags;
	auto SmartObjectSubsystem = USmartObjectSubsystem::GetCurrent(GetWorld());
	if (IsValid(SmartObjectSubsystem))
	{
		if (ActiveSOCH.IsValid())
		{
			auto SlotView = SmartObjectSubsystem->GetSlotView(ActiveSOCH.SlotHandle);
			if (SlotView.IsValid())
			{
				SlotView.GetActivityTags(InteractionTags);
				SmartObjectUserInterface->RemoveSmartObjectInteractionTags(InteractionTags);
				bInteractionTagsRemoved = true;
			}
		}
	}

	if (!bInteractionTagsRemoved)
	{
		int bDebugTrap = 1;
	}
		
	ActiveSmartObjectOwner.Reset();
	ActiveSOCH.Invalidate();
}

void USmartObjectUserInteractionComponent::ReportInteractableSmartObjectEvent(const FGameplayTag& InteractionEventTag)
{
	if (!ensure(ActiveSmartObjectOwner.IsValid()))
	{
		UE_VLOG(this, LogSmartObjectInteraction, Warning, TEXT("Tried to report event %s to smart object when no interaction was happening"), *InteractionEventTag.ToString());
		return;
	}

	ISmartObjectOwnerInteractionInterface::Execute_HandleInteractionEvent(ActiveSmartObjectOwner.Get(), InteractionEventTag);
}


