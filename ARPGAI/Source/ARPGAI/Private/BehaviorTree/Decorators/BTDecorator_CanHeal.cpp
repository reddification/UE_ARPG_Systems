#include "BehaviorTree/Decorators/BTDecorator_CanHeal.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcHealingComponent.h"

UBTDecorator_CanHeal::UBTDecorator_CanHeal()
{
	NodeName = "Can heal";
}

bool UBTDecorator_CanHeal::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	auto HealComponent = GetNpcHealComponent(OwnerComp);
	return  HealComponent ? HealComponent->CanHeal() : false;
}
