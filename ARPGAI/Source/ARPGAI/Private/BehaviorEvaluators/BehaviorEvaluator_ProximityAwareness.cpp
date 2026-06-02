#include "BehaviorEvaluators/BehaviorEvaluator_ProximityAwareness.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcProximityAwarenessComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"

UBehaviorEvaluatorConfig_ProximityAwareness::UBehaviorEvaluatorConfig_ProximityAwareness()
{
	AwarenessLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_ProximityAwareness, AwarenessLocationBBKey));
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_ProximityAwareness::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_ProximityAwareness>(*BTComponent, this);
}

FBehaviorEvaluator_ProximityAwareness::FBehaviorEvaluator_ProximityAwareness(UBehaviorTreeComponent& OwnerComp,
                                                                             const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	ProximityAwarenessConfig = Cast<UBehaviorEvaluatorConfig_ProximityAwareness>(Config);
	ensure(ProximityAwarenessConfig.IsValid());
	ProximityAwarenessComponent = Pawn->FindComponentByClass<UNpcProximityAwarenessComponent>();
	ensure(ProximityAwarenessComponent.IsValid());
	for (const auto& WakeUpNoise : ProximityAwarenessConfig->WakeUpToNoises)
		AttractingSoundsCache.AddTagFast(WakeUpNoise.Key);
}

void FBehaviorEvaluator_ProximityAwareness::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	
#if WITH_EDITOR
	if (!PerceptionComponent.IsValid())
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE, Error, TEXT("Shit's fucked up no BTMemory in proximity awareness perception update"));
		return;
	}
#endif
	
	const float RegressionOffset = GetUtilityOffset();
	float WakeUpDesire = 0.f;
	const auto& ActorsInProximity = ProximityAwarenessComponent->GetDetectedActorsInProximity();
	TArray<FProximityAwarenessCandidate, TInlineAllocator<4>> Candidates;
	
	auto DistanceDependency = ProximityAwarenessConfig->DistanceScoreDependency.GetRichCurveConst();
	FVector OwnerLocation  = Pawn->GetActorLocation();
	if (ActorsInProximity.Num() > 0)
	{
		auto ObservationDurationDependency = ProximityAwarenessConfig->TimePerceivedScoreDependency.GetRichCurveConst();
		
		for (const auto& ActorInProximity : ActorsInProximity)
		{
			const float Dist = (OwnerLocation - ActorInProximity.Key->GetActorLocation()).Size();
			const float IndividualScore = DistanceDependency->Eval(Dist) * ObservationDurationDependency->Eval(ActorInProximity.Value.Duration);
			UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Individual utility for actor %s = %.2f"), *ActorInProximity.Key->GetName(), IndividualScore);
			WakeUpDesire += IndividualScore;
			Candidates.Emplace(IndividualScore, ActorInProximity.Key.Get());
		}
	}

	auto ActorsHeardSounds = PerceptionComponent->GetHeardSounds();
	for (const auto& HeardSoundList : ActorsHeardSounds)
	{
		for (const auto& HeardSound : HeardSoundList.Value)
		{
			const bool bRelevant = !HeardSound.IsByAlly() 
				&& FMath::RandRange(0.f, 1.f) <= ProximityAwarenessConfig->WakeUpToNoiseChance;
			
			if (bRelevant)
			{
				auto MatchedSoundTag = GetMatchingSoundTag(HeardSound.SoundTag);
				if (!MatchedSoundTag.IsValid())
				{
					UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Sound %s has no suitable match. Skip"), *HeardSound.SoundTag.ToString());
					continue;
				}

				float Score = DistanceDependency->Eval(HeardSound.Distance) * ProximityAwarenessConfig->WakeUpToNoises[MatchedSoundTag];
				WakeUpDesire += Score;
				Candidates.Emplace(Score, HeardSound.Location);
				UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Individual utility for sound %s = %.2f"), *HeardSound.SoundTag.ToString(), Score);
			}
		}
	}
	
	if (GetState() == EBehaviorEvaluatorState::Activated)
	{
		if (!Candidates.IsEmpty())
		{
			if (Candidates.Num() > 1)
				Candidates.Sort();
			
			if (Candidates[0].Score > ActiveAwarenessCandidate.Score * 1.25f)
			{
				UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Updating investigation candidate [%.2f > %.2f * 1.25f]"),
					Candidates[0].Score, ActiveAwarenessCandidate.Score);
				ActiveAwarenessCandidate = Candidates[0];
				if (ensure(Blackboard.IsValid() && ProximityAwarenessConfig.IsValid()))
					Blackboard->SetValueAsVector(ProximityAwarenessConfig->AwarenessLocationBBKey.SelectedKeyName, ActiveAwarenessCandidate.Location);
			}
		}
	}
	
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Accumulated wake up desire = %.2f"), WakeUpDesire);
	InterpolateUtility(RegressionOffset + WakeUpDesire, DeltaTime);
}

void FBehaviorEvaluator_ProximityAwareness::SetState(EBehaviorEvaluatorState NewState)
{
	auto OldState = GetState();
	Super::SetState(NewState);
	if (OldState == NewState)
		return;
	
	if (Pawn.IsValid())
	{
		if (NewState == EBehaviorEvaluatorState::NotRequested || NewState == EBehaviorEvaluatorState::Blocked)
		{
			ProximityAwarenessComponent->DisableProximityAwareness();
		}
		else if (NewState == EBehaviorEvaluatorState::Relevant)
		{
			ProximityAwarenessComponent->ActivateProximityAwareness(ProximityAwarenessConfig->Radius,
				ProximityAwarenessConfig->ProximityAwarenessUpdateInterval, ProximityAwarenessConfig->ObjectTypes,
				ProximityAwarenessConfig->bIgnoreAllies, &ProximityAwarenessConfig->DetectionBlockedFilter);
		}
	}
}

void FBehaviorEvaluator_ProximityAwareness::OnActivated()
{
	Super::OnActivated();
	Update(ProximityAwarenessConfig->ProximityAwarenessUpdateInterval);
}

void FBehaviorEvaluator_ProximityAwareness::Cleanup()
{
	Super::Cleanup();
	if (Blackboard.IsValid() && ProximityAwarenessConfig.IsValid())
		Blackboard->ClearValue(ProximityAwarenessConfig->AwarenessLocationBBKey.SelectedKeyName);

	ActiveAwarenessCandidate = {};
}

FGameplayTag FBehaviorEvaluator_ProximityAwareness::GetMatchingSoundTag(const FGameplayTag& SoundTag) const
{
	if (!SoundTag.MatchesAny(AttractingSoundsCache))
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Sound %s is not interesting. Skip"), *SoundTag.ToString());
		return FGameplayTag::EmptyTag;
	}
	
	if (ProximityAwarenessConfig->WakeUpToNoises.Contains(SoundTag))
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Sound %s has direct match in attracting sounds options"), *SoundTag.ToString());
		return SoundTag;
	}
	
	FGameplayTag MatchingTag = FGameplayTag::EmptyTag;
	int MaxDepth = 0;
	for (const auto& Option : ProximityAwarenessConfig->WakeUpToNoises)
	{
		int Depth = SoundTag.MatchesTagDepth(Option.Key);
		if (Depth > MaxDepth)
		{
			MaxDepth = Depth;
			MatchingTag = Option.Key;
		}
	}
	
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Found matching sound tag for %s: %s"), *SoundTag.ToString(), *MatchingTag.ToString());
	return MatchingTag;
}