#include "Components/NpcQueueComponent.h"

#include "Components/SplineComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"
#include "Subsystems/NpcQueueSubsystem.h"

UNpcQueueComponent::UNpcQueueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNpcQueueComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!ensure(QueueId.IsValid()))
		return;

	OwnerSplineComponent = GetOwner()->FindComponentByClass<USplineComponent>();
	if (!ensure(OwnerSplineComponent.IsValid()))
		return;
	
	if (auto World = GetWorld())
		if (auto QueueSubsystem = World->GetSubsystem<UNpcQueueSubsystem>())
			QueueSubsystem->RegisterQueue(this, QueueId);

	QueuePoints.SetNum(OwnerSplineComponent->GetNumberOfSplinePoints());
	for (int i = 0; i < QueuePoints.Num(); i++)
	{
		QueuePoints[i].Location = OwnerSplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		if (i > 0)
			QueuePoints[i-1].Rotation = (QueuePoints[i-1].Location - QueuePoints[i].Location).Rotation();
	}

	QueuePoints[0].Rotation = OwnerSplineComponent->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);
	QueuePoints[0].Rotation.Yaw += 180;
	LastQueuePlaceIndex = 0;
}

void UNpcQueueComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 15.11.2024 @AK: TODO notify all active queue members (if any) that queue is unregistered
	if (auto World = GetWorld())
		if (auto QueueSubsystem = World->GetSubsystem<UNpcQueueSubsystem>())
			QueueSubsystem->UnregisterQueue(QueueId);
	
	Super::EndPlay(EndPlayReason);
}

FNpcQueueMemberPosition UNpcQueueComponent::StandInQueue(AActor* NewQueueMember)
{
	FNpcQueueMemberPosition Result;
	Result.bEntered = false;
	if (LastQueuePlaceIndex >= QueuePoints.Num())
		return Result;

	if (auto Npc = Cast<INpc>(NewQueueMember))
	{
		FGameplayTagContainer QueueTags;
		QueueTags.AddTagFast(AIGameplayTags::AI_State_Queue);
		QueueTags.AddTagFast(QueueId);
		QueueTags.AppendTags(CustomQueueTags);
		
		if (LastQueuePlaceIndex == 0)
			QueueTags.AddTag(AIGameplayTags::AI_State_Queue_First);

		Npc->GiveNpcTags(QueueTags);
	}
	
	QueuePoints[LastQueuePlaceIndex].OccupiedBy = NewQueueMember;
	// auto Npc = Cast<INpc>(NewQueueMember);
	// Npc->SetQueuePosition(QueuePoints[LastQueuePlaceIndex].Location, QueuePoints[LastQueuePlaceIndex].Rotation);

	Result.bEntered = true;
	Result.PlaceInQueue = LastQueuePlaceIndex;
	Result.QueuePointLocation = QueuePoints[LastQueuePlaceIndex].Location + FVector::UpVector * 90.f;
	Result.QueuePointRotation = QueuePoints[LastQueuePlaceIndex].Rotation;
	LastQueuePlaceIndex++;
	
	return Result;
}

FNpcQueueMemberPosition UNpcQueueComponent::GetNpcQueuePosition(const APawn* Pawn) const
{
	FNpcQueueMemberPosition Result;
	for (int i = 0; i < LastQueuePlaceIndex; i++)
	{
		if (QueuePoints[i].OccupiedBy == Pawn)
		{
			Result.bEntered = true;
			Result.PlaceInQueue = i;
			Result.QueuePointLocation = QueuePoints[i].Location;
			Result.QueuePointRotation = QueuePoints[i].Rotation;
			break;
		}
	}

	return Result;
}

void UNpcQueueComponent::LeaveQueue(AActor* LeftQueueMember)
{
	int Index = -1;
	for (int i = 0; i < LastQueuePlaceIndex; i++)
	{
		if (QueuePoints[i].OccupiedBy == LeftQueueMember)
		{
			Index = i;
			break;
		}
	}

	if (Index == -1)
		return; // this means that NPC was not in queue. can happen when loading game state
	
	if (auto Npc = Cast<INpc>(LeftQueueMember))
	{
		FGameplayTagContainer QueueTags;
		QueueTags.AddTagFast(AIGameplayTags::AI_State_Queue);
		QueueTags.AddTagFast(QueueId);
		QueueTags.AppendTags(CustomQueueTags);
		
		if (Index == 0)
			QueueTags.AddTagFast(AIGameplayTags::AI_State_Queue_First.GetTag());

		Npc->RemoveNpcTags(QueueTags);
	}

	if (Index == 0 && LastQueuePlaceIndex > 0)
		if (auto Npc = Cast<INpc>(QueuePoints[1].OccupiedBy))
			Npc->GiveNpcTags(AIGameplayTags::AI_State_Queue_First.GetTag().GetSingleTagContainer());
	
	while (Index < LastQueuePlaceIndex)
	{
		FNpcQueueMemberPosition AdvancedNpcQueueMemberPosition
		{
			QueuePoints[Index].Location,
			QueuePoints[Index].Rotation,
			Index,
			true,
		};
		
		NpcQueueMemberAdvancedEvent.Broadcast(QueuePoints[Index+1].OccupiedBy.Get(), AdvancedNpcQueueMemberPosition);
		QueuePoints[Index].OccupiedBy = QueuePoints[Index+1].OccupiedBy;
		Index++;
	}

	QueuePoints[LastQueuePlaceIndex].OccupiedBy.Reset();
	LastQueuePlaceIndex--;
}
