// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_UpdateTargetWithinNpcArea.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"

UBTService_UpdateTargetWithinNpcArea::UBTService_UpdateTargetWithinNpcArea()
{
	NodeName = "Check if target is within combat area";
	OutIsTargetOutOfNpcAreaBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTargetWithinNpcArea, OutIsTargetOutOfNpcAreaBBKey));
	TargetActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTargetWithinNpcArea, TargetActorBBKey), AActor::StaticClass());
	PredictedTargetLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTargetWithinNpcArea, PredictedTargetLocationBBKey));
	bNotifyCeaseRelevant = true;
}

void UBTService_UpdateTargetWithinNpcArea::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto NpcComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcAreasComponent>();
	if (NpcComponent == nullptr)
	{
		ensure(false);
		SetNextTickTime(NodeMemory, FLT_MAX);
		return;
	}
	
	FVector TargetLocation = FAISystem::InvalidLocation;
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorBBKey.SelectedKeyName));
	if (TargetActor != nullptr)
		TargetLocation = TargetActor->GetActorLocation();
	else
		TargetLocation = Blackboard->GetValueAsVector(PredictedTargetLocationBBKey.SelectedKeyName);

	if (TargetLocation != FVector::ZeroVector && TargetLocation != FAISystem::InvalidLocation)
		Blackboard->SetValueAsBool(OutIsTargetOutOfNpcAreaBBKey.SelectedKeyName, !NpcComponent->IsLocationWithinNpcArea(TargetLocation, AreaExtent));
	else
		Blackboard->SetValueAsBool(OutIsTargetOutOfNpcAreaBBKey.SelectedKeyName, false);
}

FString UBTService_UpdateTargetWithinNpcArea::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out]Is target out of combat zone BB: %s\n[in]Target actor BB: %s\n[in]Predicted target location BB: %s\nAllowed zone area extent: %2.f\n%s"),
		*OutIsTargetOutOfNpcAreaBBKey.SelectedKeyName.ToString(), *TargetActorBBKey.SelectedKeyName.ToString(),
		*PredictedTargetLocationBBKey.SelectedKeyName.ToString(), AreaExtent, *Super::GetStaticDescription());
}

void UBTService_UpdateTargetWithinNpcArea::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
		Blackboard->SetValueAsBool(OutIsTargetOutOfNpcAreaBBKey.SelectedKeyName, false);
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}
