


#include "Components/AIDangerousItemStimuliSourceComponent.h"

#include "Perception/AISense_Sight.h"


UAIDangerousItemStimuliSourceComponent::UAIDangerousItemStimuliSourceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoRegisterAsSource = 1;
	RegisterAsSourceForSenses.Add(UAISense_Sight::StaticClass());
}

void UAIDangerousItemStimuliSourceComponent::BeginPlay()
{
	Super::BeginPlay();
	// TODO evaluate danger
	DangerScore = 1.f;
}

float UAIDangerousItemStimuliSourceComponent::GetDangerScore() const
{
	return DangerScore;
}
