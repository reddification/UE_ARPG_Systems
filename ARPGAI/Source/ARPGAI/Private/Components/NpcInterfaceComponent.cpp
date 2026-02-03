#include "Components/NpcInterfaceComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

float UNpcInterfaceComponent::GetBaseMoveSpeed()
{
	if (auto OwnerCharacter = Cast<ACharacter>(GetOwner()))
		return OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;
	
	return GetOwner()->GetVelocity().Length();
}
