// 


#include "Components/SmartObjectOwnerInteractionComponent.h"

#include "GameplayTagContainer.h"
#include "Interfaces/SmartObjectOwnerInteractionInterface.h"


// Sets default values for this component's properties
USmartObjectOwnerInteractionComponent::USmartObjectOwnerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USmartObjectOwnerInteractionComponent::OnInteractionStarted(const FGameplayTagContainer& InSmartObjectTags,
	bool bSmartObjectActorTagsPermanent_In)
{
	auto SmartObjectOwnerInterface = Cast<ISmartObjectOwnerInteractionInterface>(GetOwner());
	if (!ensure(SmartObjectOwnerInterface))
		return;

	ISmartObjectOwnerInteractionInterface::Execute_OnInteractionStarted(GetOwner());
	if (!InSmartObjectTags.IsEmpty())
	{
		GrantedTags = InSmartObjectTags;
		bSmartObjectGrantedTagsPermanent = bSmartObjectActorTagsPermanent_In;
		SmartObjectOwnerInterface->GrantSmartObjectUsageTags(InSmartObjectTags);
	}
}

void USmartObjectOwnerInteractionComponent::OnInteractionEnded()
{
	auto SmartObjectOwnerInterface = Cast<ISmartObjectOwnerInteractionInterface>(GetOwner());
	if (!ensure(SmartObjectOwnerInterface))
		return;
	
	ISmartObjectOwnerInteractionInterface::Execute_OnInteractionEnded(GetOwner());

	if (!GrantedTags.IsEmpty() && !bSmartObjectGrantedTagsPermanent)
	{
		SmartObjectOwnerInterface->RemoveSmartObjectInteractionTags(GrantedTags);
		GrantedTags.Reset();
	}
}


