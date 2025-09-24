// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_IsPointWithinNpcArea.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcComponent.h"
#include "Interfaces/NpcZone.h"

UBTDecorator_IsPointWithinNpcArea::UBTDecorator_IsPointWithinNpcArea()
{
	NodeName = "Is point within combat zone";
	LocationBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsPointWithinNpcArea, LocationBBKey), AActor::StaticClass());
	LocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsPointWithinNpcArea, LocationBBKey));
	bNotifyBecomeRelevant = FlowAbortMode != EBTFlowAbortMode::None;
	bNotifyCeaseRelevant = FlowAbortMode != EBTFlowAbortMode::None;
}

bool UBTDecorator_IsPointWithinNpcArea::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	auto NpcComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcAreasComponent>();
	if (NpcComponent == nullptr)
		return false;

	FVector Location = FAISystem::InvalidLocation;

	if (LocationBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		auto Actor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(LocationBBKey.SelectedKeyName));
		if (Actor != nullptr)
			Location = Actor->GetActorLocation();
	}
	else if (LocationBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		Location = OwnerComp.GetBlackboardComponent()->GetValueAsVector(LocationBBKey.SelectedKeyName);
	}

	if (Location == FAISystem::InvalidLocation)
		return false;
	
	const auto& NpcAreas = NpcComponent->GetNpcAreas();
	for (const auto& NpcAreaType : NpcAreas)
		for (const auto& NpcArea : NpcAreaType.Value.NpcAreas)
			if (NpcArea->IsLocationWithinNpcArea(Location, Extent))
				return true;

	return false;
}

void UBTDecorator_IsPointWithinNpcArea::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	// TODO: register BB observer
}

void UBTDecorator_IsPointWithinNpcArea::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	// TODO: unregister BB observer
}

void UBTDecorator_IsPointWithinNpcArea::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		LocationBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTDecorator_IsPointWithinNpcArea::GetStaticDescription() const
{
	return FString::Printf(TEXT("Location BB: %s\nTest extent = %.2f"), *LocationBBKey.SelectedKeyName.ToString(), Extent);
}
