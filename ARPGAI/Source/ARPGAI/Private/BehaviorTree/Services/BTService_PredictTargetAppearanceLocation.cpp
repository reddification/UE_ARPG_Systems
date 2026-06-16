#include "BehaviorTree/Services/BTService_PredictTargetAppearanceLocation.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"

UBTService_PredictTargetAppearanceLocation::UBTService_PredictTargetAppearanceLocation()
{
	NodeName = "Predict target appearance location";
	LastKnownTargetLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PredictTargetAppearanceLocation, LastKnownTargetLocationBBKey));
	OutExpectedLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PredictTargetAppearanceLocation, OutExpectedLocationBBKey));
	bNotifyCeaseRelevant = true;
}

void UBTService_PredictTargetAppearanceLocation::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
		Blackboard->ClearValue(OutExpectedLocationBBKey.SelectedKeyName);
}

void UBTService_PredictTargetAppearanceLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto AIController = OwnerComp.GetAIOwner();
	const APawn* QuerierPawn = AIController->GetPawn();

	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_PredictTargetAppearanceLocation)
	
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	FVector PredictedLocation = Blackboard->GetValueAsVector(OutExpectedLocationBBKey.SelectedKeyName);
	FVector AssumedTargetLocation = Blackboard->GetValueAsVector(LastKnownTargetLocationBBKey.SelectedKeyName);
	
	if (!AttentionSoundTags.IsEmpty())
	{
		auto MemoryComponent = GetNpcShortTermMemoryComponent(OwnerComp);
		auto ActorsHeardSounds = MemoryComponent->GetHeardSounds();
		float ClosestSoundDistance = FLT_MAX;
		FVector ClosestSoundLocation = FAISystem::InvalidLocation;
		
		int TotalSounds = 0;
		int ValidSounds = 0;
		for (const auto& ActorHeardSounds : ActorsHeardSounds)
		{
			for (const auto& HeardSound : ActorHeardSounds.Value)
			{
				TotalSounds++;
			
				if (HeardSound.SoundTag.MatchesAny(AttentionSoundTags) && HeardSound.Distance < ClosestSoundDistance)
				{
					const bool bCanSee = (HeardSound.Traits & EPerceivedSoundTrait::CanSee) != EPerceivedSoundTrait::None;
					const bool ByAlly = (HeardSound.Traits & EPerceivedSoundTrait::ByAlly) != EPerceivedSoundTrait::None;
					const bool InFront = (HeardSound.Traits & EPerceivedSoundTrait::InFront) != EPerceivedSoundTrait::None;

					if (!bCanSee && (!ByAlly || !InFront))
					{
						ValidSounds++;
						ClosestSoundDistance = HeardSound.Distance;
						ClosestSoundLocation = HeardSound.Location;
					}
				}
			}
		}
	
		// There must be a chance for NPC to not recognize attention drawing sound if there are a lot of sounds nearby
		const bool bUseAssumedSoundLocation = ValidSounds > 0 
			&& ClosestSoundLocation != FAISystem::InvalidLocation && FMath::RandRange(0.f, 1.f) <= (0.5f + (float)ValidSounds / TotalSounds);
		if (bUseAssumedSoundLocation)
			AssumedTargetLocation = ClosestSoundLocation;
	}
	
	if (AssumedTargetLocation != FAISystem::InvalidLocation && AssumedTargetLocation != FVector::ZeroVector)
	{
		auto NavSys = UNavigationSystemV1::GetCurrent(AIController);
		FVector NpcLocation = QuerierPawn->GetActorLocation();
		if (const ANavigationData* NavData = NavSys->GetNavDataForProps(AIController->GetNavAgentPropertiesRef(), NpcLocation))
		{
			const FCollisionShape SweepShape = FCollisionShape::MakeSphere(15.f);
			FVector PathStart = AssumedTargetLocation;
			FVector PathEnd = NpcLocation;
			if (!bPathFromTargetToNpc)
			{
				PathStart = NpcLocation;
				PathEnd = AssumedTargetLocation;
			}
			
			FPathFindingQuery Query(AIController, *NavData, PathStart, PathEnd);
			Query.bAllowPartialPaths = true;
			Query.bRequireNavigableEndLocation = false;
			FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, bUseHierarchicalPathfinding ? EPathFindingMode::Hierarchical : EPathFindingMode::Regular);
			if (PathFindingResult.Result == ENavigationQueryResult::Type::Success)
			{
				const FVector SweepEnd = bPathFromTargetToNpc ? NpcLocation : AssumedTargetLocation;
				constexpr float VerticalOffset = 150.f;
				const auto& PathPoints = PathFindingResult.Path->GetPathPoints();
				for (const auto& PathPoint : PathPoints)
				{
					FHitResult Hit;
					FVector AppearLocation = PathPoint.Location + FVector::UpVector * VerticalOffset;
					
					bool bCanSee = !QuerierPawn->GetWorld()->SweepSingleByChannel(Hit, AppearLocation, SweepEnd,
						FQuat::Identity, SweepTraceChannel, SweepShape);

					if (bCanSee)
					{
						PredictedLocation = AppearLocation;
						break;
					}
				}
			}
		}		
	}
	
	UE_VLOG_LOCATION(AIController, LogARPGAI, Verbose, AssumedTargetLocation, 20, FColorList::Cyan, TEXT("Last known target location"));
	UE_VLOG_LOCATION(AIController, LogARPGAI, Verbose, PredictedLocation, 20, FColorList::GreenYellow, TEXT("Predicted location"));

	Blackboard->SetValueAsVector(OutExpectedLocationBBKey.SelectedKeyName, PredictedLocation);
}

FString UBTService_PredictTargetAppearanceLocation::GetStaticDescription() const
{
	FString PFInfo = bUseHierarchicalPathfinding ? TEXT("Use Hierarchical Pathfinding\n") : TEXT("");
	FString AttractiveSoundsList = TEXT("");
	if (!AttentionSoundTags.IsEmpty())
	{
		AttractiveSoundsList = TEXT("\nAttention sounds:\n");
		for (const auto& AttentionSound : AttentionSoundTags)
			AttractiveSoundsList += AttentionSound.ToString() + TEXT("\n");
	}
	
	return FString::Printf(TEXT("Predict target %s location\nInto %s\n%s%s%s"),
		*LastKnownTargetLocationBBKey.SelectedKeyName.ToString(), *OutExpectedLocationBBKey.SelectedKeyName.ToString(), 
		*PFInfo, *AttractiveSoundsList, *Super::GetStaticDescription());
}
