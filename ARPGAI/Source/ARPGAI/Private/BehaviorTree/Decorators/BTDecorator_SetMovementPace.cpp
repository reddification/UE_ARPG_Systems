// 


#include "BehaviorTree/Decorators/BTDecorator_SetMovementPace.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcComponent.h"

UBTDecorator_SetMovementPace::UBTDecorator_SetMovementPace()
{
	NodeName = "Set movement pace";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	bNotifyTick = true;
	bTickIntervals = true;
	FlowAbortMode = EBTFlowAbortMode::Self; // must be self in order to ::TickNode to be called
}

void UBTDecorator_SetMovementPace::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	SetNextTickTime(NodeMemory, FLT_MAX);
	auto NpcComponent = GetNpcComponent(OwnerComp);
	NpcComponent->SetMovementPaceType(MovementPaceType);
}

void UBTDecorator_SetMovementPace::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);

	FBTMemory_SetMovementPace* BTMemory = GetNodeMemory<FBTMemory_SetMovementPace>(SearchData);
	auto NpcComponent = GetNpcComponent(SearchData.OwnerComp);
	BTMemory->InitialMovementPaceType = NpcComponent->GetMovementPaceType();
	if (ActivationDelay > 0.f)
	{
		SetNextTickTime(reinterpret_cast<uint8*>(BTMemory), ActivationDelay);
	}
	else
	{
		SetNextTickTime(reinterpret_cast<uint8*>(BTMemory), FLT_MAX);
		NpcComponent->SetMovementPaceType(MovementPaceType);
	}
}

void UBTDecorator_SetMovementPace::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto NpcComponent = GetNpcComponent(SearchData.OwnerComp))
	{
		auto BTMemory = GetNodeMemory<FBTMemory_SetMovementPace>(SearchData);
		if (BTMemory->InitialMovementPaceType.IsValid())
			NpcComponent->SetMovementPaceType(BTMemory->InitialMovementPaceType);
		else 
			NpcComponent->SetMovementPaceType(FGameplayTag::EmptyTag);
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_SetMovementPace::GetStaticDescription() const
{
	FString Result = MovementPaceType.ToString();
	if (ActivationDelay > 0.f)
		Result = Result.Append(FString::Printf(TEXT("\nApply with %.2f s delay"), ActivationDelay));
	
	return Result;
}
