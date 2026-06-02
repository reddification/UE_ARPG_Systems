
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"




// Sets default values for this component's properties
UNpcBehaviorEvaluatorComponent::UNpcBehaviorEvaluatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.15f; 
}