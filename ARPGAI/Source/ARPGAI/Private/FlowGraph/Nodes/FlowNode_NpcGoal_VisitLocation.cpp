// 


#include "FlowGraph/Nodes/FlowNode_NpcGoal_VisitLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcSystemGameMode.h"

ENpcGoalStartResult UFlowNode_NpcGoal_VisitLocation::Start()
{
	auto Result = Super::Start();
	
	auto NpcInterface = Cast<INpc>(NpcPawn.Get());
	const auto& LocationTagsArray = LocationOptions.GetGameplayTagArray();
	SelectedLocationId = LocationTagsArray[FMath::RandRange(0, LocationTagsArray.Num() - 1)];
	if (bCompleteOnEntering && NpcInterface->IsAtLocation(SelectedLocationId))
		return ENpcGoalStartResult::Finished;
	
	auto NpcGameMode = Cast<INpcSystemGameMode>(BlackboardComponent->GetWorld()->GetAuthGameMode());
	if (!ensure(NpcGameMode))
		return ENpcGoalStartResult::Failed;
	
	FVector Location = NpcGameMode->GetNpcLocation(SelectedLocationId, NpcPawn->GetActorLocation(), true);
	if (Location != FAISystem::InvalidLocation)
	{
		BlackboardComponent->SetValueAsVector(BlackboardKeys->LocationToGoBBKey.SelectedKeyName, Location);
		Result = ENpcGoalStartResult::InProgress;
	}

	if (bCompleteOnEntering)
	{
		NpcFlowComponent->NpcLocationCrossedEvent.BindUObject(this, &UFlowNode_NpcGoal_VisitLocation::OnLocationCrossed);
	}
	
	return Result;
}

void UFlowNode_NpcGoal_VisitLocation::Suspend()
{
	if (NpcFlowComponent.IsValid() && NpcFlowComponent->NpcLocationCrossedEvent.IsBoundToObject(this))
		NpcFlowComponent->NpcLocationCrossedEvent.Unbind();
	
	Super::Suspend();
}

void UFlowNode_NpcGoal_VisitLocation::Finish()
{
	Super::Finish();
	SelectedLocationId = FGameplayTag::EmptyTag;
}

#if WITH_EDITOR

EDataValidationResult UFlowNode_NpcGoal_VisitLocation::ValidateNode()
{
	auto Base = Super::ValidateNode();
	if (Base == EDataValidationResult::Invalid)
		return Base;
	
	return LocationOptions.IsValid() ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}

FString UFlowNode_NpcGoal_VisitLocation::GetGoalDescription() const
{
	auto Base = Super::GetGoalDescription();
	Base = Base + FString::Printf(TEXT("\nLocation: %s"), *LocationOptions.ToStringSimple());
	if (bCompleteOnEntering)
		Base += TEXT("\nComplete on entering");
	
	return Base;
}

#endif

void UFlowNode_NpcGoal_VisitLocation::OnLocationCrossed(const FGameplayTag& CrossedLocationId, bool bEntered)
{
	if (CrossedLocationId == SelectedLocationId && bEntered)
	{
		TriggerOutput("Completed", true);
		NpcFlowComponent->OnGoalCompleted(CustomGoalTags, FGameplayTagContainer::EmptyContainer);
	}
}
