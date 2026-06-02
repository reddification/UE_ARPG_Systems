// 


#include "FlowGraph/Addons/FlowNodeAddon_SetRoleplayObjective.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/RoleplayComponent.h"

void UFlowNodeAddon_SetRoleplayObjective::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	if (!ObjectiveTag.IsValid())
		return;
	
	auto Owner = Cast<AAIController>(TryGetRootFlowActorOwner());
	if (URoleplayComponent* RoleplayComponent = GetRoleplayComponent(Owner))
	{
		PreviousObjectiveTag = RoleplayComponent->GetObjective();
		RoleplayComponent->SetObjective(ObjectiveTag);
	}
}

void UFlowNodeAddon_SetRoleplayObjective::FinishState()
{
	if (auto Owner = Cast<AAIController>(TryGetRootFlowActorOwner()))
		if (URoleplayComponent* RoleplayComponent = GetRoleplayComponent(Owner))
			RoleplayComponent->SetObjective(PreviousObjectiveTag);
	
	Super::FinishState();
}
