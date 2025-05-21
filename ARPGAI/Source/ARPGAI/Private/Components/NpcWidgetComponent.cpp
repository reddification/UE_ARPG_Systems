

#include "Components/NpcWidgetComponent.h"

void UNpcWidgetComponent::InitWidget()
{
	Super::InitWidget();
	OnWidgetComponentInitialized.ExecuteIfBound();
}
