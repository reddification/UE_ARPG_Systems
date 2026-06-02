#include "BehaviorEvaluators/BehaviorEvaluator_Investigate.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcActorTagsInterface.h"

UBehaviorEvaluatorConfig_Investigate::UBehaviorEvaluatorConfig_Investigate()
{
	InvestigateLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Investigate, InvestigateLocationBBKey));
	// AnchorLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Investigate, AnchorLocationBBKey));
	InvestigateActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Investigate, InvestigateActorBBKey), AActor::StaticClass());
	InvestigationDurationBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Investigate, InvestigationDurationBBKey));
	InvestigationTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Investigate, InvestigationTagsBBKey)));
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Investigate::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Investigate>(*BTComponent, this);
}

FBehaviorEvaluator_Investigate::FBehaviorEvaluator_Investigate(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config)
	: Super(OwnerComp, Config)
{
	InvestigateConfig = Cast<UBehaviorEvaluatorConfig_Investigate>(Config);
	for (const auto& AttractingSoundOption : InvestigateConfig->AttractingSoundsOptions)
		AttractingSoundsCache.AddTag(AttractingSoundOption.Key);
}

void FBehaviorEvaluator_Investigate::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	float NewUtility = UpdatePerception();
	InterpolateUtility(GetUtilityOffset() + NewUtility, DeltaTime);
}

float FBehaviorEvaluator_Investigate::UpdatePerception()
{
	const auto& ActorHeardSounds = PerceptionComponent->GetHeardSounds();
	const auto& CharacterPerceptionContainer = PerceptionComponent->GetShortTermCharactersMemory();
	
	float InvestigationDesire = 0.f;
	TArray<FInvestigationCandidate, TInlineAllocator<6>> InvestigationCandidates;
	auto DistanceDependency = InvestigateConfig->AttractingSoundToDistanceDependencyCurve.GetRichCurveConst();
	FGameplayTagContainer InvestigationTags;
	for (const auto& HeardSounds : ActorHeardSounds)
	{
		for (const auto& HeardSound : HeardSounds.Value)
		{
			auto MatchedSoundTag = GetMatchingSoundTag(HeardSound.SoundTag);
			if (!MatchedSoundTag.IsValid())
			{
				UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Sound %s has no suitable match. Skip"), *HeardSound.SoundTag.ToString());
				continue;
			}
		
			const auto& SoundParams = InvestigateConfig->AttractingSoundsOptions[MatchedSoundTag];
			if (IsSoundIgnored(MatchedSoundTag, SoundParams, HeardSound.Location))
			{
				UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Sound %s is ignored. Skip"), *HeardSound.SoundTag.ToString());
				continue;
			}
		
			const float TraitsScale = GetTraitsScale(SoundParams.TraitScales, HeardSound.Traits);
			if (TraitsScale == 0.f)
			{
				UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Sound %s skipped because traits checks scaled it to zero"),
					*HeardSound.SoundTag.ToString());
				continue;
			}
		
			float LocalUtilityAccumulation = SoundParams.Score * HeardSound.Loudness * TraitsScale * DistanceDependency->Eval(HeardSound.Distance);

			InvestigationCandidates.Add(FInvestigationCandidate(LocalUtilityAccumulation, HeardSound.Location, HeardSounds.Key.Get(),
				MatchedSoundTag));
			InvestigationTags.AddTag(MatchedSoundTag);
			InvestigationDesire += LocalUtilityAccumulation;
			UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Sound %s [matched %s] is interesting [%.2f]"), *HeardSound.SoundTag.ToString(),
				*MatchedSoundTag.ToString(), LocalUtilityAccumulation);
		}
	}
	
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Accumulated investigation desire from sounds = %.2f"), InvestigationDesire);

	if (!InvestigateConfig->ActorInterests.IsEmpty())
	{
		if (!CharacterPerceptionContainer.IsEmpty())
		{
			const auto* DistanceDependencyCurve = InvestigateConfig->InterestingActorDistanceDependencyCurve.GetRichCurveConst();
			for (const auto& CharacterData : CharacterPerceptionContainer)
			{
				if (!CharacterData.Value.HasVisualDetection())
					continue;
				
				for (const auto& Interest : InvestigateConfig->ActorInterests)
				{
					if (Interest.TagsQuery.IsEmpty() && Interest.RelevantAttitudes.IsEmpty() || !CharacterData.Key.IsValid())
					{
						ensure(false);
						continue;
					}
					
					if (Interest.RelevantAttitudes.IsValid() && !CharacterData.Value.Attitude.MatchesAny(Interest.RelevantAttitudes))
						continue;
					
					if (!Interest.TagsQuery.IsEmpty() && !Interest.TagsQuery.Matches(CharacterData.Value.CharacterTags))
						continue;
					
					float ActorInterestScore = DistanceDependencyCurve->Eval(CharacterData.Value.Distance) * Interest.ScoreScale;
					InvestigationCandidates.Add(FInvestigationCandidate(ActorInterestScore, CharacterData.Key.Get(), Interest.InterestCause));
					InvestigationDesire += ActorInterestScore;
					if (Interest.InterestCause.IsValid())
						InvestigationTags.AddTag(Interest.InterestCause);
					
					UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Actor %s is interesting [%.2f] for interest cause [%s]"),
						*CharacterData.Key->GetName(), ActorInterestScore, *Interest.InterestCause.ToString());
					
					break;
				}
			}
		}
	}
	
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Accumulated investigation desire from sounds and actors = %.2f"), InvestigationDesire);
	
	if (GetState() == EBehaviorEvaluatorState::Activated && !InvestigationCandidates.IsEmpty())
	{
		if (InvestigationCandidates.Num() > 1)
			InvestigationCandidates.Sort();
		
		const FInvestigationCandidate& BestCandidate = InvestigationCandidates[0];
		const bool bUpdateActiveInvestigation = !ActiveInvestigation.IsValid() 
			|| BestCandidate.Score > ActiveInvestigation.Score * InvestigateConfig->ActiveInvestigationPrioritizationScale;
		if (bUpdateActiveInvestigation)
		{
			UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Updating invesitgation:\nPrevious: %s\nNew: %s"),
				*ActiveInvestigation.ToString(), *BestCandidate.ToString());
			const bool bPreviousInvestigationUrgent = ActiveInvestigation.bUrgent;
			ActiveInvestigation = BestCandidate;
			if (InvestigationCandidates[0].InvestigationCause == FInvestigationCandidate::EInvestigationCause::Actor)
			{
				Blackboard->SetValueAsObject(InvestigateConfig->InvestigateActorBBKey.SelectedKeyName, InvestigationCandidates[0].Actor.Get());
				Blackboard->ClearValue(InvestigateConfig->InvestigateLocationBBKey.SelectedKeyName);
				InvestigationTags.AddTag(AIGameplayTags::Behavior_Investigation_Cause_Actor);
			}
			else if (InvestigationCandidates[0].InvestigationCause == FInvestigationCandidate::EInvestigationCause::Sound)
			{
				Blackboard->SetValueAsObject(InvestigateConfig->InvestigateActorBBKey.SelectedKeyName, InvestigationCandidates[0].Actor.Get());
				Blackboard->SetValueAsVector(InvestigateConfig->InvestigateLocationBBKey.SelectedKeyName, InvestigationCandidates[0].Location);
				InvestigationTags.AddTag(AIGameplayTags::Behavior_Investigation_Cause_Location);
			}

			const bool bNewInvestigationUrgent = InvestigateConfig->UrgentInvestigationStateTag.IsValid() 
				&& !InvestigateConfig->UrgentInvestigationReasons.IsEmpty() 
				&& InvestigationTags.HasAny(InvestigateConfig->UrgentInvestigationReasons);
			ActiveInvestigation.bUrgent = bNewInvestigationUrgent;
			if (bNewInvestigationUrgent && !bPreviousInvestigationUrgent)
			{
				if (auto Npc = Cast<INpcActorTagsInterface>(Pawn.Get()))
					Npc->GiveTag_NPC(InvestigateConfig->UrgentInvestigationStateTag);
			}
			else if (bPreviousInvestigationUrgent && !bNewInvestigationUrgent)
			{
				if (auto Npc = Cast<INpcActorTagsInterface>(Pawn.Get()))
					Npc->RemoveTag_NPC(InvestigateConfig->UrgentInvestigationStateTag);
			}
			
			// Blackboard->SetValueAsVector(InvestigateConfig->AnchorLocationBBKey.SelectedKeyName, BestCandidate.Location);
			Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(InvestigateConfig->InvestigationTagsBBKey.SelectedKeyName, InvestigationTags);
		}
	}
	
	return InvestigationDesire;
}

FGameplayTag FBehaviorEvaluator_Investigate::GetMatchingSoundTag(const FGameplayTag& SoundTag) const
{
	if (!SoundTag.MatchesAny(AttractingSoundsCache))
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Sound %s is not interesting. Skip"), *SoundTag.ToString());
		return FGameplayTag::EmptyTag;
	}
	
	if (InvestigateConfig->AttractingSoundsOptions.Contains(SoundTag))
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Sound %s has direct match in attracting sounds options"), *SoundTag.ToString());
		return SoundTag;
	}
	
	FGameplayTag MatchingTag = FGameplayTag::EmptyTag;
	int MaxDepth = 0;
	for (const auto& Option : InvestigateConfig->AttractingSoundsOptions)
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

bool FBehaviorEvaluator_Investigate::IsSoundIgnored(const FGameplayTag& SoundTag,
                                                    const FSoundInvestigationParams& AttractingSoundParams, const FVector& TestLocation) const
{
	if (!AttractingSoundParams.bCanIgnore)
		return false;
	
	const auto* IgnoredSoundLocations = IgnoredSoundsLocations.Find(SoundTag);
	if (IgnoredSoundLocations == nullptr)
		return false;
	
	const FDateTime DateTimeNow = GetGameTime();
	const float IgnoredSoundRange = AttractingSoundParams.IgnoreRadius;
	const float IgnoredSoundRangeSq = IgnoredSoundRange * IgnoredSoundRange;
	for (const auto& IgnoredSoundLocation : (*IgnoredSoundLocations))
	{
		if (IgnoredSoundLocation.UntilGameTime >= DateTimeNow && (TestLocation - IgnoredSoundLocation.Location).SizeSquared() <= IgnoredSoundRangeSq)
		{
			UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Ignoring sound %s because it is in ignored sounds list until %s (now is %s)"),
				*SoundTag.ToString(), *IgnoredSoundLocation.UntilGameTime.ToFormattedString(TEXT("%j:%H:%M:%S")), *DateTimeNow.ToFormattedString(TEXT("%j:%H:%M:%S")));
			UE_VLOG_LOCATION(AIController.Get(), LogARPGAI_BE, VeryVerbose, IgnoredSoundLocation.Location, IgnoredSoundRange, FColorList::LightGrey, TEXT("Ignored sound %s area"), *SoundTag.ToString());
			UE_VLOG_LOCATION(AIController.Get(), LogARPGAI_BE, VeryVerbose, TestLocation, 20, FColorList::BrightGold, TEXT("Ignored sound %s probe"), *SoundTag.ToString());
			return true;
		}
	}
	
	return false;
}

float FBehaviorEvaluator_Investigate::GetTraitsScale(const TArray<FSoundEventTraitInvestigationScale>& TraitChecks,
	EPerceivedSoundTrait Traits) const
{
	float Scale = 1.f;
	if (TraitChecks.IsEmpty())
		return Scale;
	
	for (const auto& TraitCheck : TraitChecks)
	{
		bool bHasTrait = (Traits & TraitCheck.TestTrait) != EPerceivedSoundTrait::None;
		if (bHasTrait == TraitCheck.bCheckPresent)
			Scale *= TraitCheck.Scale;
	}
	
	return Scale;
}

void FBehaviorEvaluator_Investigate::OnActivated()
{
	Super::OnActivated();
	UpdatePerception();
	float InvestigationDuration = FMath::RandRange(InvestigateConfig->InvestigationDurationInterval.Min, InvestigateConfig->InvestigationDurationInterval.Max);
	Blackboard->SetValueAsFloat(InvestigateConfig->InvestigationDurationBBKey.SelectedKeyName, InvestigationDuration);
	TArray<FGameplayTag, TInlineAllocator<4>> ClearedIgnoredSounds;
	
	FDateTime GameTimeNow = GetGameTime();
	for (auto& IgnoredSoundLocations : IgnoredSoundsLocations)
	{
		for (int i = IgnoredSoundLocations.Value.Num() - 1; i >= 0; --i)
			if (IgnoredSoundLocations.Value[i].UntilGameTime <= GameTimeNow)
				IgnoredSoundLocations.Value.RemoveAt(i);
		
		if (IgnoredSoundLocations.Value.IsEmpty())
			ClearedIgnoredSounds.Add(IgnoredSoundLocations.Key);
	} 
	
	UE_VLOG(AIController.Get(), LogARPGAI, Log, TEXT("Cleared %d ignored sounds"), ClearedIgnoredSounds.Num());
	for (const auto& ClearedIgnoredSound : ClearedIgnoredSounds)
		IgnoredSoundsLocations.Remove(ClearedIgnoredSound);
}

void FBehaviorEvaluator_Investigate::Cleanup()
{
	Super::Cleanup();
	
	if (!InvestigateConfig.IsValid())
		return;
	
	if (ActiveInvestigation.IsValid() && ActiveInvestigation.bUrgent)
	{
		if (auto Npc = Cast<INpcActorTagsInterface>(Pawn.Get()))
			Npc->RemoveTag_NPC(InvestigateConfig->UrgentInvestigationStateTag);
	}
	
	if (Blackboard.IsValid())
	{
		Blackboard->ClearValue(InvestigateConfig->InvestigateLocationBBKey.SelectedKeyName);
		Blackboard->ClearValue(InvestigateConfig->InvestigationDurationBBKey.SelectedKeyName);
		Blackboard->ClearValue(InvestigateConfig->InvestigateActorBBKey.SelectedKeyName);
		Blackboard->ClearValue(InvestigateConfig->InvestigationTagsBBKey.SelectedKeyName);
		// Blackboard->ClearValue(InvestigateConfig->AnchorLocationBBKey.SelectedKeyName);
		ActiveInvestigation = {};
	}
}

void FBehaviorEvaluator_Investigate::HandleMessage_Internal(const FGameplayTag& MessageTag)
{
	Super::HandleMessage_Internal(MessageTag);
	if (MessageTag == AIGameplayTags::BrainMessage_Investigate_Event_Completed)
	{
		if (ActiveInvestigation.IsValidLocationEvent())
		{
			const auto* SoundParams = InvestigateConfig->AttractingSoundsOptions.Find(ActiveInvestigation.EventTag);
			if (ensure(SoundParams) && SoundParams->bCanIgnore)
			{
				auto& IgnoredSoundsContainer = IgnoredSoundsLocations.FindOrAdd(ActiveInvestigation.EventTag);
				IgnoredSoundsContainer.Add(FIgnoredSound(ActiveInvestigation.Location, GetGameTime(SoundParams->IgnoreDurationGTH)));
				UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Ignoring sound %s for %.2f GTH"), 
					*ActiveInvestigation.EventTag.ToString(), SoundParams->IgnoreDurationGTH);
			}
			
			Blackboard->ClearValue(InvestigateConfig->InvestigateLocationBBKey.SelectedKeyName);
			Blackboard->ClearValue(InvestigateConfig->InvestigateActorBBKey.SelectedKeyName);
		}
		else if (ActiveInvestigation.IsValidActorEvent())
		{
			Blackboard->ClearValue(InvestigateConfig->InvestigateActorBBKey.SelectedKeyName);
			// Blackboard->ClearValue(InvestigateConfig->AnchorLocationBBKey.SelectedKeyName);
		}
		
		ActiveInvestigation = {};
	}
}

FString FBehaviorEvaluator_Investigate::FInvestigationCandidate::ToString() const
{
	return FString::Printf(TEXT("%s [%.2f]"), Actor.IsValid() ? *Actor->GetName() : *Location.ToString(), Score);
}

bool FBehaviorEvaluator_Investigate::FInvestigationCandidate::IsValidActorEvent() const
{
	return InvestigationCause == EInvestigationCause::Actor && EventTag.IsValid();
}

bool FBehaviorEvaluator_Investigate::FInvestigationCandidate::IsValidLocationEvent() const
{
	return InvestigationCause == EInvestigationCause::Sound && EventTag.IsValid() && FAISystem::IsValidLocation(Location);
}
