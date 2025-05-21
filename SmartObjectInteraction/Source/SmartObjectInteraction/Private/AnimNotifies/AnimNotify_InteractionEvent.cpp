// 


#include "AnimNotifies/AnimNotify_InteractionEvent.h"

#include "Components/SmartObjectUserInteractionComponent.h"
#include "GameFramework/Character.h"

void UAnimNotify_InteractionEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	auto Owner = MeshComp->GetOwner();
	if (!Owner || !Owner->IsA<ACharacter>())
		return;

	if (!ensure(InteractionEventTag.IsValid()))
		return;
		
	auto SmartObjectUserInteractionComponent = Owner->FindComponentByClass<USmartObjectUserInteractionComponent>();
	if (ensure(SmartObjectUserInteractionComponent))
		SmartObjectUserInteractionComponent->ReportInteractableSmartObjectEvent(InteractionEventTag);
}
