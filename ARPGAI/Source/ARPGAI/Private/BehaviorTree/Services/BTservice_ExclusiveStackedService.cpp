#include "BehaviorTree/Services/BTservice_ExclusiveStackedService.h"

#include "Components/Controller/EnhancedBehaviorTreeComponent.h"

UBTservice_ExclusiveStackedService::UBTservice_ExclusiveStackedService()
{
	NodeName = "Exclusive stacked service";
}

void UBTservice_ExclusiveStackedService::Freeze(uint8* NodeMemory)
{
	SetNextTickTime(NodeMemory, FLT_MAX);
}

void UBTservice_ExclusiveStackedService::Unfreeze(uint8* NodeMemory)
{
	SetNextTickTime(NodeMemory, 0.f);
}

void UBTservice_ExclusiveStackedService::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	ScheduleNextTick(OwnerComp, NodeMemory);
	if (UEnhancedBehaviorTreeComponent* EnhancedBTComponent = Cast<UEnhancedBehaviorTreeComponent>(&OwnerComp))
		EnhancedBTComponent->AddStackedService(StackItemKey, this);
}

void UBTservice_ExclusiveStackedService::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UEnhancedBehaviorTreeComponent* EnhancedBTComponent = Cast<UEnhancedBehaviorTreeComponent>(&OwnerComp))
		EnhancedBTComponent->RemoveStackedService(StackItemKey, this);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}
