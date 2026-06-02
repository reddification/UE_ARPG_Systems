#include "Components/RoleplayComponent.h"

URoleplayComponent::URoleplayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void URoleplayComponent::SetPersonality(const FGameplayTag& NewPersonality)
{
	IdentityPersonalityTag = NewPersonality;
}

void URoleplayComponent::SetTemper(const FGameplayTag& NewTemperTag)
{
	IdentityTemperTag = NewTemperTag;
}

void URoleplayComponent::SetObjective(const FGameplayTag& NewObjectiveTag)
{
	CurrentObjectiveTag = NewObjectiveTag;
}
