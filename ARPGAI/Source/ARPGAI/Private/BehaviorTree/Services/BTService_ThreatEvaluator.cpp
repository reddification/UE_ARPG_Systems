
#include "BehaviorTree/Services/BTService_ThreatEvaluator.h"
#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/AIDangerousItemStimuliSourceComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcPerceptionComponent.h"
#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "GameFramework/Character.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Threat.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Settings/NpcCombatSettings.h"

UBTService_ThreatEvaluator::UBTService_ThreatEvaluator()
{
	NodeName = "Threat Evaluator";
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ThreatEvaluator, TargetBBKey), AActor::StaticClass());
	EvaluationIntervalBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ThreatEvaluator, EvaluationIntervalBBKey));
	OutTargetThreatLevelBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTService_ThreatEvaluator, OutTargetThreatLevelBBKey)));
	OutInvestigationLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ThreatEvaluator, OutInvestigationLocationBBKey));
	bNotifyBecomeRelevant = 1;
	bNotifyCeaseRelevant = 1;
}

void UBTService_ThreatEvaluator::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	Interval = 0.2f; 
	FBTCombatEvaluatorNodeMemory* CombatEvaluatorNodeMemory = reinterpret_cast<FBTCombatEvaluatorNodeMemory*>(NodeMemory);
	CombatEvaluatorNodeMemory->UpdateInterval = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(EvaluationIntervalBBKey.SelectedKeyName);
	auto NpcPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (UNpcComponent* NpcCombatComponent = NpcPawn->FindComponentByClass<UNpcComponent>())
	{
		if (const auto MobDTR = NpcCombatComponent->GetNpcDTR())
		{
			CombatEvaluatorNodeMemory->DamageScoreFactor = MobDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.DamageScoreFactor;
			CombatEvaluatorNodeMemory->UpdateInterval = MobDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.UpdateInterval;
			CombatEvaluatorNodeMemory->bIgnoreTeamDamage = MobDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.bIgnoreTeamDamage;
			CombatEvaluatorNodeMemory->MobCoordinatorScoreFactor = MobDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.MobCoordinatorScoreFactor;
			CombatEvaluatorNodeMemory->TeammateTargetScoreFactor = MobDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.TeammateTargetScoreFactor;
		}
	}

	if (const INpcAliveCreature* AliveCreature = Cast<INpcAliveCreature>(NpcPawn))
	{
		CombatEvaluatorNodeMemory->MaxHealth = AliveCreature->GetMaxHealth();		
	}

	
	EvaluateThreats(OwnerComp, CombatEvaluatorNodeMemory);
}

void UBTService_ThreatEvaluator::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTCombatEvaluatorNodeMemory* ProtectZoneMemory = reinterpret_cast<FBTCombatEvaluatorNodeMemory*>(NodeMemory);
	ProtectZoneMemory->bHadTarget = false;
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_ThreatEvaluator::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	FBTCombatEvaluatorNodeMemory* ProtectZoneMemory = reinterpret_cast<FBTCombatEvaluatorNodeMemory*>(NodeMemory);
	EvaluateThreats(OwnerComp, ProtectZoneMemory);
}

void UBTService_ThreatEvaluator::EvaluateThreats(UBehaviorTreeComponent& OwnerComp, FBTCombatEvaluatorNodeMemory* CombatEvaluatorNodeMemory)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_ThreatEvaluator::EvaluateThreats)
	
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ThisNpc = AIController->GetPawn();
	UNpcComponent* NpcComponent = ThisNpc->FindComponentByClass<UNpcComponent>();
	auto NpcCombatLogicComponent = ThisNpc->FindComponentByClass<UNpcCombatLogicComponent>();
	
	const FNpcDTR* MobDTR = NpcComponent->GetNpcDTR();
	
	FCombatEvaluationParameters CombatEvaluationParameters(AIController, NpcComponent, NpcCombatLogicComponent,
		Cast<const UNpcPerceptionComponent>(AIController->GetAIPerceptionComponent()), ThisNpc, CombatEvaluatorNodeMemory->MaxHealth);

	UE_VLOG(AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Evaluating threats"));
	
	FCombatEvaluationResult CombatEvaluationResult;
	TMap<FGameplayTag, float> PreviousBehaviorUtilitiesScores;
	PreviousBehaviorUtilitiesScores.Reserve(BehaviorUtilitiesBBKeys.Num());
	{
		int i = 0;
		for (const auto& BehaviorUtilityType : BehaviorUtilitiesBBKeys)
		{
			PreviousBehaviorUtilitiesScores.Emplace(BehaviorUtilityType.Key, CombatEvaluatorNodeMemory->BehaviorUtilities[i]);
			UE_VLOG(AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Previous behavior %s utility = %.2f"),
				*BehaviorUtilityType.Key.ToString(), CombatEvaluatorNodeMemory->BehaviorUtilities[i]);
			
			++i;
		}
	}
	
	TMap<FGameplayTag, float> BehaviorUtilitiesScores;
	BehaviorUtilitiesScores.Reserve(MobDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.CombatBehaviorUtilityParameters.Num());
	FGameplayTag PreviousBestBehaviorTypeTag = FGameplayTag::EmptyTag;
	float PreviousBestUtilityScore = TNumericLimits<float>::Lowest();
	FGameplayTag NewBestBehaviorTypeTag = FGameplayTag::EmptyTag;
	float NewBestUtilityScore = TNumericLimits<float>::Lowest();
	
	ProcessPerception(CombatEvaluationParameters, CombatEvaluatorNodeMemory, CombatEvaluationResult);
	ReceiveTeammateAwareness(CombatEvaluationResult, CombatEvaluationParameters, CombatEvaluatorNodeMemory);
	EvaluateBehaviorUtilities(CombatEvaluatorNodeMemory, ThisNpc, MobDTR, CombatEvaluationResult, BehaviorUtilitiesScores);
	GetBestBehaviorUtility(BehaviorUtilitiesScores, NewBestBehaviorTypeTag, NewBestUtilityScore);
	GetBestBehaviorUtility(PreviousBehaviorUtilitiesScores, PreviousBestBehaviorTypeTag, PreviousBestUtilityScore);

	UE_VLOG(AIController, LogARPGAI_ThreatEvaluator, Verbose, TEXT("Previous best utility = %.2f %s"), PreviousBestUtilityScore, *PreviousBestBehaviorTypeTag.ToString());
	UE_VLOG(AIController, LogARPGAI_ThreatEvaluator, Verbose, TEXT("New best utility = %.2f %s"), NewBestUtilityScore, *NewBestBehaviorTypeTag.ToString());
	
	bool bUpdateBB = PreviousBestUtilityScore == TNumericLimits<float>::Lowest()
		|| (FMath::Abs(PreviousBestUtilityScore - NewBestUtilityScore) > BehaviourUtilityDifferenceThreshold);
	if (bUpdateBB)
	{
		UE_VLOG(AIController, LogARPGAI_ThreatEvaluator, Verbose, TEXT("Updating blackboard utilities"));
		int i = 0;
		for (const auto& BehaviorUtilityTypeBBKey : BehaviorUtilitiesBBKeys)
		{
			UE_VLOG(AIController, LogARPGAI_ThreatEvaluator, Verbose, TEXT("Blackboard: %s utility = %.2f"), *BehaviorUtilityTypeBBKey.Key.ToString(),
				BehaviorUtilitiesScores[BehaviorUtilityTypeBBKey.Key]);
			BlackboardComponent->SetValueAsFloat(BehaviorUtilityTypeBBKey.Value.SelectedKeyName, BehaviorUtilitiesScores[BehaviorUtilityTypeBBKey.Key]);
			CombatEvaluatorNodeMemory->BehaviorUtilities[i++] = BehaviorUtilitiesScores[BehaviorUtilityTypeBBKey.Key];
		}		
	}

	CombatEvaluatorNodeMemory->CurrentBehaviorTag = NewBestBehaviorTypeTag;
	bool bHasTarget = false;
	if (NewBestBehaviorTypeTag != FGameplayTag::EmptyTag)
	{
		bHasTarget = AssignBestTarget(NpcCombatLogicComponent, BlackboardComponent, CombatEvaluatorNodeMemory, CombatEvaluationResult, NewBestBehaviorTypeTag);
	}
	
	if (bHasTarget && CombatEvaluatorNodeMemory->bHadTarget == false)
	{
		CombatEvaluatorNodeMemory->bHadTarget = true;
		Interval = CombatEvaluatorNodeMemory->UpdateInterval;
	}
	
	SetActiveThreats(NpcCombatLogicComponent, CombatEvaluationResult);
	if (CombatEvaluationResult.OutInvestigationLocation != FVector::ZeroVector && CombatEvaluationResult.OutInvestigationLocation != FAISystem::InvalidLocation)
	{
		UE_VLOG(AIController, LogARPGAI_ThreatEvaluator, Verbose, TEXT("Setting investigation location"));
		UE_VLOG_LOCATION(AIController, LogARPGAI_ThreatEvaluator, Verbose, CombatEvaluationResult.OutInvestigationLocation, 12, FColor::White, TEXT("Investigation location"));
		BlackboardComponent->SetValueAsVector(OutInvestigationLocationBBKey.SelectedKeyName, CombatEvaluationResult.OutInvestigationLocation);
	}
}

void UBTService_ThreatEvaluator::ProcessDangerousItemPerception(FCombatEvaluationResult& OutResult, AActor* TargetActor, const FVector& MobLocation) const
{
	if (const UAIDangerousItemStimuliSourceComponent* DangerSourceComponent = TargetActor->FindComponentByClass<UAIDangerousItemStimuliSourceComponent>())
	{
		const float DangerScore = DangerSourceComponent->GetDangerScore();
		ProcessDangerousItemPerception(OutResult, TargetActor, DangerScore, MobLocation, Visual);
	}
}

void UBTService_ThreatEvaluator::ProcessDangerousItemPerception(FCombatEvaluationResult& OutResult, AActor* Target,
	const float DangerousItemScore, const FVector& MobLocation, EDetectionSource DetectionSource) const
{
	FNpcCombatPerceptionData DangerousItemPerceptionData;
	DangerousItemPerceptionData.PerceptionScore = DangerousItemScore;
	DangerousItemPerceptionData.Distance = FVector::Distance(Target->GetActorLocation(), MobLocation);
	DangerousItemPerceptionData.AddDetectionSource(DetectionSource);
	OutResult.DangerousActors.Add(Target, DangerousItemPerceptionData);
}

bool UBTService_ThreatEvaluator::IsHostile(const FCombatEvaluationParameters& Parameters, AActor* Actor, EDetectionSource DetectionSource) const
{
	const FGameplayTag& Attitude = Parameters.NpcComponent->GetAttitude(Actor);
	return DetectionSource == EDetectionSource::Damage && Attitude != AIGameplayTags::AI_Attitude_Friendly
		|| Attitude != AIGameplayTags::AI_Attitude_Neutral && Attitude != AIGameplayTags::AI_Attitude_Friendly; 
}

void UBTService_ThreatEvaluator::ProcessPerception(const FCombatEvaluationParameters& Parameters, const FBTCombatEvaluatorNodeMemory* NodeMemory,
                                                   FCombatEvaluationResult& OutResult) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_ThreatEvaluator::ProcessPerception)
	
	const APawn* NpcPawn = Parameters.AIController->GetPawn();
	const FAISenseID HearingSenseID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());

	if (Parameters.NpcComponent->IsAlive() == false)
	{
		UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, Verbose, TEXT("Not processing perception: NPC is not alive"));
		return;
	}

	float NpcMaxHealth = Parameters.MaxHealth;
	
	TMap<FVector, float> HearingEventsLocations;
	const FVector& NpcLocation = NpcPawn->GetActorLocation();
	const UNpcCombatSettings* NpcCombatSettings = GetDefault<UNpcCombatSettings>();
	for (auto DataIt = Parameters.PerceptionComponent->GetPerceptualDataConstIterator(); DataIt; ++DataIt)
	{
		if (DataIt->Value.Target.IsValid())
			UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Processing perception for %s"), *DataIt->Value.Target->GetName());

		// scan hearing and damage, collect visual targets
		for (const auto& AIStimulus : DataIt.Value().LastSensedStimuli)
		{
			if (AIStimulus.IsExpired())
			{
				UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Stimulus of type %s is expired for %s"),
					 *AIStimulus.Type.Name.ToString(), *DataIt->Value.Target->GetName());
				
				continue;
			}

			if (AIStimulus.Type == HearingSenseID && DataIt->Value.Target != Parameters.Npc)
			{
				FGameplayTag SoundTag = FGameplayTag::RequestGameplayTag(AIStimulus.Tag, true);
				UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Perceived sound event %s"), *SoundTag.ToString());
				UE_VLOG_LOCATION(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, AIStimulus.StimulusLocation, 25, FColor::Yellow, TEXT("%s"), *SoundTag.ToString());
				
				if (SoundTag == AIGameplayTags::AI_Noise_EnemySpotted)
				{
					auto AllyNpc = Cast<APawn>(DataIt->Value.Target.Get());
					if (!AllyNpc)
						continue;
					
					auto AllyNpcComponent = AllyNpc->FindComponentByClass<UNpcCombatLogicComponent>();
					if (!AllyNpcComponent)
						continue;

					const auto& AllyThreats = AllyNpcComponent->GetActiveThreats();
					if (AllyThreats.IsEmpty())
						continue;
					
					for (const auto& AllyThreat : AllyThreats)
					{
						// TODO do something smart
						// *response to myself few weeks later* do what? i don't even know what I meant there...
						if (AllyThreat.Key.IsValid())
						{
							HearingEventsLocations.Add(AllyThreat.Key->GetActorLocation(), AIStimulus.Strength);
							UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Heard info about threat from ally %s"), *AllyNpc->GetName());
							UE_VLOG_LOCATION(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, AllyThreat.Key->GetActorLocation(), 25, FColor::Yellow, TEXT("Threat heard from ally"));
						}
					}
				}
				else if (NpcCombatSettings->DangerousSounds.HasTag(SoundTag))
				{
					UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Heard dangerous sound"));
					HearingEventsLocations.Add(AIStimulus.StimulusLocation, AIStimulus.Strength);
				}
			}
			else if (AActor* TargetActor = DataIt->Value.Target.Get())
			{
				UE_VLOG_LOCATION(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TargetActor->GetActorLocation(), 25, FColor::White, TEXT("Perceived %s"), *TargetActor->GetName());

				EDetectionSource DetectionSource = AIStimulus.Type == UAISense::GetSenseID(UAISense_Damage::StaticClass())
					? EDetectionSource::Damage
					: EDetectionSource::Visual;
				if (IsHostile(Parameters, TargetActor, DetectionSource))
				{
					ACharacter* PerceivedCharacter = Cast<ACharacter>(TargetActor);
					if (PerceivedCharacter != nullptr)
					{
						if (auto AliveCreature = Cast<INpcAliveCreature>(PerceivedCharacter))
						{
							if (AliveCreature->IsNpcActorAlive())
								ProcessCharacterPerception(Parameters, NodeMemory, OutResult, NpcMaxHealth, AIStimulus, PerceivedCharacter);	
							else
								UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Perceived %s but NPC it's not alive"), *TargetActor->GetName());
						}
					}
					else
					{
						ProcessDangerousItemPerception(OutResult, TargetActor, NpcLocation);
					}	
				}
				else
				{
					UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Perceived %s but NPC is not hostile to it"), *TargetActor->GetName());
				}
			}
		}
	}

	// pick best hearing target
	float LoudestHearingEventStrength = 0.f;
	for (const auto& InterestingLocation : HearingEventsLocations)
	{
		if (InterestingLocation.Value > LoudestHearingEventStrength)
		{
			LoudestHearingEventStrength = InterestingLocation.Value;
			OutResult.OutInvestigationLocation = InterestingLocation.Key;
		}
	}

	if (OutResult.OutInvestigationLocation != FVector::ZeroVector && OutResult.OutInvestigationLocation != FAISystem::InvalidLocation)
	{
		UE_VLOG_LOCATION(Parameters.AIController, LogARPGAI_ThreatEvaluator, Verbose, OutResult.OutInvestigationLocation, 12, FColor::Cyan, TEXT("Investigation location"));
	}
}

void UBTService_ThreatEvaluator::ProcessCharacterPerception(const FCombatEvaluationParameters& Parameters, const FBTCombatEvaluatorNodeMemory* NodeMemory,
    FCombatEvaluationResult& OutResult, float MobMaxHealth, const FAIStimulus& AIStimulus, ACharacter* PerceivedCharacter) const
{
	if (AIStimulus.Type == UAISense::GetSenseID(UAISense_Damage::StaticClass()))
	{
		if (Parameters.NpcCombatLogicComponent->TryForgiveReceivingDamage(PerceivedCharacter))
			return;
		
		FNpcCombatPerceptionData& PlayerPerceptionData = OutResult.DangerousActors.FindOrAdd(PerceivedCharacter);
		PlayerPerceptionData.bCharacter = true;
		PlayerPerceptionData.AddDetectionSource(Damage);
		PlayerPerceptionData.Distance = FVector::Distance(PerceivedCharacter->GetActorLocation(), Parameters.AIController->GetPawn()->GetActorLocation());;
		PlayerPerceptionData.PerceptionScore += AIStimulus.Strength / MobMaxHealth * NodeMemory->DamageScoreFactor;
		Parameters.NpcComponent->SetHostile(PerceivedCharacter);
		OutResult.AccumulatedDamagePercent += AIStimulus.Strength / MobMaxHealth;

		UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Perceived damage from %s. Perception score = %.2f, Accumumulated damage = %.2f"),
			*PerceivedCharacter->GetName(), PlayerPerceptionData.PerceptionScore, OutResult.AccumulatedDamagePercent);

	}
	else if (AIStimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
	{
		ProcessCharacterVisualPerception(Parameters, OutResult, PerceivedCharacter);
	}
}

void UBTService_ThreatEvaluator::ProcessCharacterVisualPerception(const FCombatEvaluationParameters& Parameters, FCombatEvaluationResult& OutResult,
                                                                  ACharacter* PerceivedCharacter) const
{
	const float AccumulatedTimeSeen = Parameters.PerceptionComponent->GetAccumulatedTimeSeen(PerceivedCharacter);
	if (AccumulatedTimeSeen < TimeSeenToReact)
	{
		UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Not processing visual perception for %s because time seen < threshold (%.2f < %.2f)"),
			*PerceivedCharacter->GetName(), AccumulatedTimeSeen, TimeSeenToReact);
		return;
	}

	FGameplayTag Attitude = Parameters.NpcComponent->GetAttitude(PerceivedCharacter);
	if (!Attitude.MatchesTag(AIGameplayTags::AI_Attitude_Hostile))
	{
		UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Not processing visual perception for %s because attitude is not hostile (%s)"),
			*PerceivedCharacter->GetName(), *Attitude.ToString());
		return;
	}
	
	const FVector& NpcLocation = Parameters.AIController->GetPawn()->GetActorLocation();
	FVector PerceivedCharacterLocation = PerceivedCharacter->GetActorLocation();
	FNpcCombatPerceptionData& CharacterPerceptionData = OutResult.DangerousActors.FindOrAdd(PerceivedCharacter);
	CharacterPerceptionData.bCharacter = true;
	CharacterPerceptionData.AddDetectionSource(Visual);
	CharacterPerceptionData.Distance = FVector::Distance(NpcLocation, PerceivedCharacterLocation);
	if (auto Threat = Cast<IThreat>(PerceivedCharacter))
	{
		CharacterPerceptionData.ThreatScore = Threat->GetThreatLevel(Parameters.Npc);	
	}
	
	CharacterPerceptionData.PerceptionScore = CharacterPerceptionData.ThreatScore;
	UE_VLOG(Parameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Registered visual perception for %s; Threat score = %.2f, Perception score = %.2f"),
		*PerceivedCharacter->GetName(), CharacterPerceptionData.ThreatScore, CharacterPerceptionData.PerceptionScore);
}

void UBTService_ThreatEvaluator::EvaluateBehaviorUtilities(const FBTCombatEvaluatorNodeMemory* CombatEvaluatorNodeMemory,
	const APawn* ThisNpc, const FNpcDTR* NpcDTR, FCombatEvaluationResult& CombatEvaluationResult, TMap<FGameplayTag, float>& BehaviorUtilitiesScores)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_ThreatEvaluator::EvaluateBehaviorUtilities)
	
	for (const auto& ReactionUtilityData : BehaviorUtilitiesBBKeys)
	{
		const auto* NpcReactionParameters = NpcDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.CombatBehaviorUtilityParameters.Find(ReactionUtilityData.Key);
		if (NpcReactionParameters)
		{
			float ReactionUtility = EvaluateBehaviorUtility(CombatEvaluationResult, *NpcReactionParameters, ThisNpc,
				CombatEvaluatorNodeMemory, ReactionUtilityData.Key);
			
			UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("%s utility = %.2f"), *ReactionUtilityData.Key.ToString(), ReactionUtility);
			BehaviorUtilitiesScores.Emplace(ReactionUtilityData.Key, ReactionUtility);
		}
		else
		{
			UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, Warning, TEXT("No reaction evaluating parameters for %s"), *ReactionUtilityData.Key.ToString());
			ensure(false);
		}
	}
}

void UBTService_ThreatEvaluator::GetBestBehaviorUtility(const TMap<FGameplayTag, float>& BehaviorUtilities,
	FGameplayTag& OutBestBehaviorTypeTag, float& OutBestBehaviorUtility) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_ThreatEvaluator::GetBestBehaviorUtility)
	
	OutBestBehaviorUtility = TNumericLimits<float>::Lowest();
	OutBestBehaviorTypeTag = FGameplayTag::EmptyTag;
	for (const auto& BehaviorUtility : BehaviorUtilities)
	{
		if (BehaviorUtility.Value > OutBestBehaviorUtility)
		{
			OutBestBehaviorTypeTag = BehaviorUtility.Key;
			OutBestBehaviorUtility = BehaviorUtility.Value;
		}
	}
}

// TODO refactor because combat and retreat evaluation begin to diverge immensely 
float UBTService_ThreatEvaluator::EvaluateBehaviorUtility(FCombatEvaluationResult& CombatEvaluationResult,
	const FBehaviorUtilityParameters& BehaviorUtilityParameters, const APawn* ThisNpc, const FBTCombatEvaluatorNodeMemory* NodeMemory,
	const FGameplayTag& BehaviorUtilityTag) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_ThreatEvaluator::EvaluateBehaviorUtility)
	
	float ThreatWeight = 0.f;
	// TODO filter character dangerous actors by their LoS (no retreat utility wait when player is not seeing this mob but maybe more combat utility)
	// TODO consider different stimuli types: combat-visual perception should scale closer targets more, retreat-damage should scale remotest targets the most
	for (auto& DangerousItem : CombatEvaluationResult.DangerousActors)
	{
		UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Evaluating threat for %s (behavior %s)"), *DangerousItem.Key->GetName(), *BehaviorUtilityTag.ToString());
		
		bool bThreatIsFacingAway = false;
		if (DangerousItem.Value.bCharacter && BehaviorUtilityTag == AIGameplayTags::AI_Behavior_Combat_Retreat
			&& DangerousItem.Value.DetectionSource == EDetectionSource::Visual)
		{
			const float TargetToNpcDotProduct = FVector::DotProduct(DangerousItem.Key->GetActorForwardVector(),
				(ThisNpc->GetActorLocation() - DangerousItem.Key->GetActorLocation()).GetSafeNormal());
			if (TargetToNpcDotProduct < ThreatToNpcDotProductIgnoreThreat)
			{
				UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("%s is looking away from NPC, ignoring retreat threat (dp = %.2f < %.2f threshold)"),
					*DangerousItem.Key->GetName(), TargetToNpcDotProduct, ThreatToNpcDotProductIgnoreThreat);
				bThreatIsFacingAway = true;
			}	
		}
		
		float Awareness = 1.f;
		float UtilityScore = DangerousItem.Value.PerceptionScore;
		
		for (const auto& ActorTypeAwareness : BehaviorUtilityParameters.ActorTypeAwarenesses)
		{
			UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Actor type awareness base score for %s = %.2f (behavior %s)"),
				*DangerousItem.Key->GetName(), ActorTypeAwareness.Value.BaseScore, *BehaviorUtilityTag.ToString());

			if (DangerousItem.Key->IsA(ActorTypeAwareness.Key))
			{
				Awareness = ActorTypeAwareness.Value.BaseScore;
				
				UtilityScore *= Awareness;
				if (const FRichCurve* Curve = ActorTypeAwareness.Value.DistanceAwarenessScaleDependencyCurve.GetRichCurveConst())
				{
					UtilityScore *= Curve->Eval(DangerousItem.Value.Distance);
				}

				UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("%s utility score for %s = %.2f"),
					*BehaviorUtilityTag.ToString(), *DangerousItem.Key->GetName(), UtilityScore);
				
				break;
			}
		}

		if (BehaviorUtilityTag == AIGameplayTags::AI_Behavior_Combat)
		{
			if (UEnemiesCoordinatorComponent* TargetEnemiesCoordinatorComponent = DangerousItem.Key->FindComponentByClass<UEnemiesCoordinatorComponent>())
			{
				int CurrentTargetAttackersCount = TargetEnemiesCoordinatorComponent->GetAttackersCount(ThisNpc);
				UtilityScore /= (1.f + CurrentTargetAttackersCount * NodeMemory->MobCoordinatorScoreFactor);
			}

			UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Post enemies coordinator component correction: %s utility score for %s = %.2f"),
				*BehaviorUtilityTag.ToString(), *DangerousItem.Key->GetName(), UtilityScore);
		}

		DangerousItem.Value.BehaviorUtilityScores.FindOrAdd(BehaviorUtilityTag) += UtilityScore; 
		ThreatWeight += bThreatIsFacingAway ? 0.f : Awareness;

		UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Accumulated threat weight = %.2f (behavior %s"), ThreatWeight, *BehaviorUtilityTag.ToString());
	}

	float DamageUtilityScore = 0.f;
	if (const FRichCurve* AccumulatedDamageReactionDependency = BehaviorUtilityParameters.AccumulatedDamageReactionDependency.GetRichCurveConst())
	{
		DamageUtilityScore = AccumulatedDamageReactionDependency->Eval(CombatEvaluationResult.AccumulatedDamagePercent);
		UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Damage utility score = %.2f (behavior %s"), DamageUtilityScore, *BehaviorUtilityTag.ToString());
	}

	// TODO store TotalNoiseLoudness in CombatEvaluationResult?
	float PerceivedThreatsUtilityScore = 0.f;
	if (const FRichCurve* PerceivedThreatsWeightReactionDependency = BehaviorUtilityParameters.PerceivedThreatsWeightReactionDependency.GetRichCurveConst())
	{
		PerceivedThreatsUtilityScore = PerceivedThreatsWeightReactionDependency->Eval(ThreatWeight);
		UE_VLOG(ThisNpc, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Perceived threats utility score = %.2f (behavior %s"), PerceivedThreatsUtilityScore, *BehaviorUtilityTag.ToString());
	}
	
	return DamageUtilityScore + PerceivedThreatsUtilityScore;
}

void UBTService_ThreatEvaluator::SetActiveThreats(UNpcCombatLogicComponent* NpcCombatComponent, const FCombatEvaluationResult& CombatEvaluationResult) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_ThreatEvaluator::SetActiveThreats)

	UE_VLOG(NpcCombatComponent->GetOwner(), LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Setting active threats"));
	
	TMap<TWeakObjectPtr<AActor>, FNpcThreatData> Threats;
	FGameplayTag RetreatBehaviorUtilityTag = AIGameplayTags::AI_Behavior_Combat_Retreat;
	Threats.Reserve(CombatEvaluationResult.DangerousActors.Num());
	for (const auto& DangerousItem : CombatEvaluationResult.DangerousActors)
	{
		FNpcThreatData ThreatData;
		if (const float* RetreatBehaviorUtilityScore = DangerousItem.Value.BehaviorUtilityScores.Find(RetreatBehaviorUtilityTag))
		{
			ThreatData.RetreatUtilityScore = *RetreatBehaviorUtilityScore;
		}

		ThreatData.ThreatScore = DangerousItem.Value.ThreatScore;
		if (auto ThreatInterface = Cast<IThreat>(DangerousItem.Key))
		{
			ThreatData.AttackRange = ThreatInterface->GetAttackRange();	
		}
		
		Threats.Emplace(DangerousItem.Key, ThreatData);
	}

	NpcCombatComponent->SetActiveThreats(Threats);
}

void UBTService_ThreatEvaluator::ReceiveTeammateAwareness(FCombatEvaluationResult& CombatEvaluationResult,
                                                          const FCombatEvaluationParameters& CombatEvaluationParameters,
                                                          const FBTCombatEvaluatorNodeMemory* CombatEvalulatorNodeMemory) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_ThreatEvaluator::ReceiveTeammateAwareness)
	
	TArray<APawn*> Allies = CombatEvaluationParameters.NpcCombatLogicComponent->GetAllies(false);
	const int RandomTeammatesCount = Allies.Num() / 2;
	if (Allies.Num() > 2)
	{
		for (int i = 0; i < Allies.Num(); i++)
		{
			int IndexToSwap = Allies[i] == CombatEvaluationParameters.Npc ? Allies.Num() - 1 : FMath::RandRange(0, Allies.Num() - 2);
			Allies.Swap(i, IndexToSwap);
		}	
	}
	
	for (int i = 0; i < RandomTeammatesCount; i++)
	{
		if (Allies[i] == CombatEvaluationParameters.Npc)
			continue; // huh?

		UE_VLOG(CombatEvaluationParameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Processing teammate awareness from %s"), *Allies[i]->GetOwner()->GetName());
		
		UNpcCombatLogicComponent* AllyNpcCombatComponent = Allies[i]->FindComponentByClass<UNpcCombatLogicComponent>();
		// if NpcCombatComponent contained a TMap<TWeakObjectPtr<const AActor*>, FPerceptionData> we could actually share all threats info between mobs
		const FNpcActiveTargetData& AllyBestTargetPerception = AllyNpcCombatComponent->GetCurrentCombatTarget();
		if (AActor* SharedTarget = AllyBestTargetPerception.ActiveTarget.Get())
		{
			UE_VLOG(CombatEvaluationParameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Ally has target %s"), *SharedTarget->GetName());
			UE_VLOG_LOCATION(CombatEvaluationParameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose, SharedTarget->GetActorLocation(), 12,
				FColor::Emerald, TEXT("Ally target"));

			if (auto NpcAliveTarget = Cast<INpcAliveCreature>(SharedTarget))
				if (!NpcAliveTarget->IsNpcActorAlive())
					continue;

			if (CombatEvaluationParameters.NpcComponent->GetAttitude(SharedTarget).MatchesTag(AIGameplayTags::AI_Attitude_Friendly))
				continue;
			
			if (FNpcCombatPerceptionData* MyCommonTargetPerception = CombatEvaluationResult.DangerousActors.Find(SharedTarget))
			{
				MyCommonTargetPerception->PerceptionScore = FMath::Max(MyCommonTargetPerception->PerceptionScore,
					AllyBestTargetPerception.NpcCombatPerceptionData.PerceptionScore * CombatEvalulatorNodeMemory->TeammateTargetScoreFactor);
				UE_VLOG(CombatEvaluationParameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose,
					TEXT("I also have this target, so taking max of perception scores: %.2f (source = %.2f)"),
					MyCommonTargetPerception->PerceptionScore, AllyBestTargetPerception.NpcCombatPerceptionData.PerceptionScore);
				for (auto& SharedBehaviorUtilityScore : AllyBestTargetPerception.NpcCombatPerceptionData.BehaviorUtilityScores)
				{
					float& MyScore = MyCommonTargetPerception->BehaviorUtilityScores.FindOrAdd(SharedBehaviorUtilityScore.Key);
					MyScore = FMath::Max(MyScore,SharedBehaviorUtilityScore.Value * CombatEvalulatorNodeMemory->TeammateTargetScoreFactor);
					UE_VLOG(CombatEvaluationParameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose,
						TEXT("I also have this target, so taking max of behavior utility scores: %.2f [%s] (source = %.2f)"),
						MyScore, *SharedBehaviorUtilityScore.Key.ToString(), SharedBehaviorUtilityScore.Value);
				}

				MyCommonTargetPerception->AddDetectionSource(Teammate);
			}
			else
			{
				FNpcCombatPerceptionData SharedPerception = AllyBestTargetPerception.NpcCombatPerceptionData;
				SharedPerception.AddDetectionSource(Teammate);
				SharedPerception.PerceptionScore *= CombatEvalulatorNodeMemory->TeammateTargetScoreFactor;

				UE_VLOG(CombatEvaluationParameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose,
					TEXT("I don't have this target, but taking it with perception score: %.2f (against %.2f from source)"),
						SharedPerception.PerceptionScore, AllyBestTargetPerception.NpcCombatPerceptionData.PerceptionScore);

				for (auto& SharedBehaviorUtilityScore : SharedPerception.BehaviorUtilityScores)
				{
					SharedBehaviorUtilityScore.Value *= CombatEvalulatorNodeMemory->TeammateTargetScoreFactor;

					UE_VLOG(CombatEvaluationParameters.AIController, LogARPGAI_ThreatEvaluator, VeryVerbose,
					TEXT("I don't have this target, but taking it with behavior utility score: %.2f (against %.2f from source)"),
						SharedBehaviorUtilityScore.Value, SharedBehaviorUtilityScore.Value / CombatEvalulatorNodeMemory->TeammateTargetScoreFactor);
				}
				
				CombatEvaluationResult.DangerousActors.Add(SharedTarget, SharedPerception);
			}
		}
	}
}

bool UBTService_ThreatEvaluator::AssignBestTarget(UNpcCombatLogicComponent* NpcCombatComponent, UBlackboardComponent* BlackboardComponent,
                                                  FBTCombatEvaluatorNodeMemory* NodeMemory, FCombatEvaluationResult& CombatEvaluationResult, const FGameplayTag& BestBehaviorUtilityTag) const
{
	UE_VLOG(BlackboardComponent->GetOwner(), LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("Assigning best target"));
	
	AActor* BestTarget = nullptr;
	float PreviousTargetCurrentScore = TNumericLimits<float>::Lowest();
	float BestScore = -1.f;
	float BestTargetThreat = 0.f;
	for (auto& DangerousItem : CombatEvaluationResult.DangerousActors)
	{
		if (NodeMemory->CurrentTarget == DangerousItem.Key)
		{
			PreviousTargetCurrentScore = DangerousItem.Value.BehaviorUtilityScores[BestBehaviorUtilityTag];
		}
		
		if (DangerousItem.Value.BehaviorUtilityScores[BestBehaviorUtilityTag] > BestScore)
		{
			BestScore = DangerousItem.Value.BehaviorUtilityScores[BestBehaviorUtilityTag];
			BestTarget = DangerousItem.Key;
			BestTargetThreat = DangerousItem.Value.ThreatScore;
		}
	}	

	bool bTargetChanged = BestScore - PreviousTargetCurrentScore >= BestTargetScoreDifferenceThreshold;
	if (!bTargetChanged)
	{
		UE_VLOG(BlackboardComponent->GetOwner(), LogARPGAI_ThreatEvaluator, VeryVerbose,
			TEXT("Not changing best target, because target score doesn't change enough from previous target: %.2f - %.2f < %.2f (threshold)"),
			BestScore, PreviousTargetCurrentScore, BestTargetScoreDifferenceThreshold);
		BestTarget = NodeMemory->CurrentTarget.Get();
	}
	
	if (BestTarget != nullptr)
	{
		UE_VLOG(BlackboardComponent->GetOwner(), LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("New target: %s, Score = %.2f"), *BestTarget->GetName(), BestScore);
		NodeMemory->CurrentTarget = BestTarget;
		NpcCombatComponent->SetCurrentCombatTarget(BestTarget, *CombatEvaluationResult.DangerousActors.Find(BestTarget), BestBehaviorUtilityTag);
		if (bTargetChanged)
		{
			BlackboardComponent->SetValueAsObject(TargetBBKey.SelectedKeyName, const_cast<AActor*>(BestTarget));

			FGameplayTagContainer ThreatLevelTagContainer;
			FGameplayTag ThreatLevelTag = NpcCombatComponent->GetThreatLevel(BestTargetThreat);
			UE_VLOG(BlackboardComponent->GetOwner(), LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("New target threat level: %s"), *ThreatLevelTag.ToString());

			ThreatLevelTagContainer.AddTagFast(ThreatLevelTag);
			BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(OutTargetThreatLevelBBKey.SelectedKeyName, ThreatLevelTagContainer);
			NpcCombatComponent->SetEnemyThreatLevel(ThreatLevelTag);
		}
	}
	else
	{
		UE_VLOG(BlackboardComponent->GetOwner(), LogARPGAI_ThreatEvaluator, VeryVerbose, TEXT("No target, clearing blacbkoard"));

		BlackboardComponent->ClearValue(TargetBBKey.SelectedKeyName);
		BlackboardComponent->ClearValue(OutTargetThreatLevelBBKey.SelectedKeyName);
		NpcCombatComponent->ResetCurrentCombatTarget();
		NodeMemory->CurrentTarget.Reset();
	}

	return BestTarget != nullptr;
}

FGameplayTag UBTService_ThreatEvaluator::GetBestBehaviorTag(const TMap<FGameplayTag, float>& BehaviorUtilitiesScores) const
{
	FGameplayTag BestBehaviorTypeTag = FGameplayTag::EmptyTag;
	float BestBehaviorUtilityScore = TNumericLimits<float>::Lowest();
	for (const auto& BehaviorUtilityScore : BehaviorUtilitiesScores)
	{
		if (BehaviorUtilityScore.Value > BestBehaviorUtilityScore)
		{
			BestBehaviorTypeTag = BehaviorUtilityScore.Key;
			BestBehaviorUtilityScore = BehaviorUtilityScore.Value;
		}
	}

	return BestBehaviorTypeTag;
}

FString UBTService_ThreatEvaluator::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT(
		"%s\nTarget BB: %s\nEvaluation Interval BB: %s"
		"\nUtility difference threshold = %.2f\nBest target score difference threshold = %.2f"
		"\n[out]Investigation location BB: %s"
		"\nIgnore threat from player at dot product: %.2f"),
		*Super::GetStaticDescription(), *TargetBBKey.SelectedKeyName.ToString(),
		*EvaluationIntervalBBKey.SelectedKeyName.ToString(),
		BehaviourUtilityDifferenceThreshold, BestTargetScoreDifferenceThreshold, *OutInvestigationLocationBBKey.SelectedKeyName.ToString(),
		ThreatToNpcDotProductIgnoreThreat);

	for (const auto& BehaviorUtilityBBKey : BehaviorUtilitiesBBKeys)
	{
		Result = Result.Append(FString::Printf(TEXT("\n%s utility BB: %s"), *BehaviorUtilityBBKey.Key.ToString(), *BehaviorUtilityBBKey.Value.SelectedKeyName.ToString()));
	}
	
	return Result;
}

void UBTService_ThreatEvaluator::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	FBTCombatEvaluatorNodeMemory* CombatEvaluatorNodeMemory = reinterpret_cast<FBTCombatEvaluatorNodeMemory*>(NodeMemory);
	
}

uint16 UBTService_ThreatEvaluator::GetInstanceMemorySize() const
{
	return sizeof(FBTCombatEvaluatorNodeMemory);
}

void UBTService_ThreatEvaluator::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BBAsset = Asset.GetBlackboardAsset())
	{
		for (auto& ReactionUtilityBBKey : BehaviorUtilitiesBBKeys)
		{
			ReactionUtilityBBKey.Value.ResolveSelectedKey(*BBAsset);
			if (ReactionUtilityBBKey.Value.SelectedKeyName != NAME_None)
			{
				FName FilterName = FName(FString::Printf(TEXT("%s_%s"),
					*GET_MEMBER_NAME_CHECKED(UBTService_ThreatEvaluator, BehaviorUtilitiesBBKeys).ToString(),
					*ReactionUtilityBBKey.Value.SelectedKeyName.ToString()));
				ReactionUtilityBBKey.Value.AddFloatFilter(this, FilterName);
			}
		}
	}
}