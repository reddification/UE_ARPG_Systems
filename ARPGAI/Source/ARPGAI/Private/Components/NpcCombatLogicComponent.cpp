// 


#include "Components/NpcCombatLogicComponent.h"

#include "AIController.h"
#include "GameplayEffectExtension.h"
#include "Components/EnhancedBehaviorTreeComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Threat.h"
#include "Perception/AISense_Damage.h"
#include "Settings/NpcCombatSettings.h"
#include "Subsystems/NpcActivitySquadSubsystem.h"

// Called when the game starts
void UNpcCombatLogicComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerNPC.SetObject(GetOwner());
	OwnerNPC.SetInterface(Cast<INpc>(GetOwner()));

	NpcDTRH = OwnerNPC->GetNpcDataTableRowHandle();
	
	OwnerAliveCreature.SetObject(GetOwner());
	OwnerAliveCreature.SetInterface(Cast<INpcAliveCreature>(GetOwner()));
	OwnerAliveCreature->OnDeathStarted.AddUObject(this, &UNpcCombatLogicComponent::OnNpcDeathStarted);

	AttackRangeScale = GetDefault<UNpcCombatSettings>()->AIAttackRangeScale;
	
	OwnerPawn = Cast<APawn>(GetOwner());

	if (auto AIController = Cast<AAIController>(OwnerPawn->GetController()); ensure(AIController))
		InitializeNpcCombatLogic(*AIController);
}

void UNpcCombatLogicComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromDelegates();
	
	Super::EndPlay(EndPlayReason);
}

void UNpcCombatLogicComponent::InitializeCombatData()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::InitializeCombatData)
	
	if (UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (auto NpcCombatAttributeSet = ASC->GetSet<UNpcCombatAttributeSet>())
		{
			InitializeNpcCombatAttributeSet(ASC, NpcCombatAttributeSet);
		}

		auto HealthAttribute = OwnerAliveCreature->GetHealthAttribute();
		ASC->GetGameplayAttributeValueChangeDelegate(HealthAttribute).AddUObject(this, &UNpcCombatLogicComponent::OnHealthChanged);
			SetHealth(ASC->GetNumericAttribute(HealthAttribute));

		auto StaminaAttribute = OwnerAliveCreature->GetStaminaAttribute();
		ASC->GetGameplayAttributeValueChangeDelegate(StaminaAttribute).AddUObject(this, &UNpcCombatLogicComponent::OnStaminaChanged);
			SetStamina(ASC->GetNumericAttribute(StaminaAttribute));
	}

	auto NpcDTR = GetNpcDTR();
	if (const auto DistanceIntelligenceDependency = NpcDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.IntelligenceAttackRangeDeviationDependency.GetRichCurveConst())
	{
		DistanceIntelligenceDependcyFactorCachedIntelligence = Intelligence;
		DistanceIntelligenceDependcyFactor = DistanceIntelligenceDependency->Eval(Intelligence);
	}
	
	ForgivableCountOfHitsForAttitude = NpcDTR->NpcCombatParametersDataAsset->ForgivableCountOfReceivedHits;
}

void UNpcCombatLogicComponent::InitializeNpcCombatAttributeSet(UAbilitySystemComponent* ASC, const UNpcCombatAttributeSet* NpcCombatAttributeSet)
{
	ASC->GetGameplayAttributeValueChangeDelegate(OwnerNPC->GetAttackRangeAttribute())
	   .AddUObject(this, &UNpcCombatLogicComponent::OnAttackRangeChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetAggressionAttribute())
	   .AddUObject(this, &UNpcCombatLogicComponent::OnAggressivenessChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetIntellectAttribute())
	   .AddUObject(this, &UNpcCombatLogicComponent::OnIntellectChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetReactionAttribute())
	   .AddUObject(this, &UNpcCombatLogicComponent::OnReactionChanged);
	
	SetAttackRange(OwnerNPC->GetAttackRange());
	SetSurroundRange(NpcCombatAttributeSet->GetSurroundRange());
	SetAggression(NpcCombatAttributeSet->GetAggression());
	SetIntelligence(NpcCombatAttributeSet->GetIntellect());
	SetReaction(NpcCombatAttributeSet->GetReaction());
}

float UNpcCombatLogicComponent::GetTauntProbabilityOnSuccessfulAttack() const
{
	if (auto NpcDTR = GetNpcDTR())
	{
		if (NpcDTR->NpcCombatParametersDataAsset)
		{
			IThreat* ActiveEnemy = OwnerNPC->GetActiveTarget();
			float EnemyHealth = ActiveEnemy->GetHealth();
			const auto& NpcCombatEvaluationParameters = NpcDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters;
			bool bShouldTaunt = ActiveEnemy->IsStaggered();
			if (!bShouldTaunt)
			{
				const float TrueDistanceBetweenEnemyAndNpc = (ActiveEnemy->GetThreatLocation() - GetOwner()->GetActorLocation()).Size();
				float PerceivedDistanceBetweenEnemyAndNpc = GetIntellectAffectedDistance(TrueDistanceBetweenEnemyAndNpc);
				bShouldTaunt = PerceivedDistanceBetweenEnemyAndNpc > GetIntellectAffectedDistance(AttackRange + ActiveEnemy->GetAttackRange()) * NpcCombatEvaluationParameters.SafeDistanceForTauntFactor;
			}

			bShouldTaunt &= Health / EnemyHealth > NpcCombatEvaluationParameters.NpcToEnemyHealthRatioToTauntStaggeredEnemy;
			bShouldTaunt |= FMath::RandRange(0.f, NpcCombatEvaluationParameters.ChanceToTauntEnemyInBadScenario) > Intelligence; 
			
			return bShouldTaunt ? NpcDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.TauntChanceAfterAttack : 0.f;
			// TODO FMath::Min(StaminaDependency->Eval(Stamina), AggressionDependency->Eval(Aggression))
		}
	}

	ensure(false);
	return 0.f;
}

float UNpcCombatLogicComponent::GetBackstepProbabilityOnWhiff() const
{
	if (auto NpcDTR = GetNpcDTR())
	{
		if (ensure(NpcDTR->NpcCombatParametersDataAsset))
		{
			if (auto BackstepStaminaProbabilityDependency = NpcDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.BackstepStaminaProbabilityDependency.GetRichCurve())
			{
				return BackstepStaminaProbabilityDependency->Eval(Stamina);
			}

			// TODO consider dependency from stamina
			// TODO FMath::Min(StaminaDependency->Eval(Stamina), AggressionDependency->Eval(Aggression))
		}
	}

	ensure(false);
	return 0.f;
}

void UNpcCombatLogicComponent::SetEnemyThreatLevel(const FGameplayTag& InThreatLevelTag)
{
	ActiveThreatLevelTag = InThreatLevelTag;
	auto NpcCombatDTR = GetNpcDTR();
	if (InThreatLevelTag.IsValid())
	{
		if (float* MinAnxietyLevel = NpcCombatDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.PerceivedThreatToMinAnxiety.Find(InThreatLevelTag))
		{
			
		}
		else
		{
			// ensure(false);			
		}
	}
	else
	{
		// TODO start reducing anxiety
	}
}

void UNpcCombatLogicComponent::OnBlockCompleted()
{
	ResetReactionToIncomingAttack();
}

void UNpcCombatLogicComponent::OnDodgeCompleted()
{
	ResetReactionToIncomingAttack();
}

bool UNpcCombatLogicComponent::TryForgiveReceivingDamage(const APawn* DamageCauser)
{
	// 24.12.2024 @AK TODO refactor getting attitude 
	FGameplayTag Attitude = GetOwner()->FindComponentByClass<UNpcComponent>()->GetAttitude(DamageCauser);
	int* ForgivableCountOfHits = ForgivableCountOfHitsForAttitude.Find(Attitude);
	if (ForgivableCountOfHits == nullptr)
		return false;
	
	int& ReceivedHitsFromCharacterCount = ReceivedHitsCountFromCharacters.FindOrAdd(DamageCauser, 0);
	ReceivedHitsFromCharacterCount++;
	bool bForgive = ReceivedHitsFromCharacterCount <= *ForgivableCountOfHits;
	return bForgive;
}

void UNpcCombatLogicComponent::ReactToIncomingAttack(AActor* ThreatActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::ReactToIncomingAttack)
	
	auto ThreatData = ActiveThreats.Find(ThreatActor);
	if (!ensure(ThreatData))
		return;
	
	auto NpcCombatDTR = GetNpcDTR();
	// @AK: FYI this formula is out of my ass. feel free to adjust if the parry chance seems to be too rare
	const float ChanceToReact = FMath::Clamp(
		(Reaction + Intelligence * 0.75f - Aggressiveness * 0.25f) * Stamina / OwnerAliveCreature->GetMaxStamina(),
		0.25f,
		0.9f);
	if (FMath::RandRange(0.f, 1.f) > ChanceToReact)
	{
		UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("Didn't react to incoming attack from %s because reaction check failed"), *ThreatActor->GetName());
		// AddIgnoredIncomingAttackFromThreat(ThreatActor, 0.25f);
		return; // if reaction check fails - "miss" an attacking threat
	}

	const float PerceivedDistance = GetIntellectAffectedDistance((GetOwner()->GetActorLocation() - ThreatActor->GetActorLocation()).Size());
	const float PerceivedEnemyAttackRange = GetIntellectAffectedDistance(ThreatData->AttackRange + 80.f); // 80 is approximate attack step range
	
	if (PerceivedDistance > PerceivedEnemyAttackRange)
	{
		UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("Didn't react to incoming attack from %s because perceived attack range is bigger than attackers range"), *ThreatActor->GetName());
		return;	
	}

	ENpcDefensiveAction RecommendedAction = ENpcDefensiveAction::Parry;
	if (ActiveReactToAttackActor.IsValid())
	{
		RecommendedAction = ENpcDefensiveAction::Dodge;
	}
	else
	{
		RecommendedAction = ENpcDefensiveAction::Parry;
		if (PerceivedDistance + NpcCombatDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.StepOutDistance > PerceivedEnemyAttackRange)
		{
			float BackstepProbability = 0.5f;
			if (auto BackstepReactionDependency = NpcCombatDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.BackstepReactionProbabilityDependency.GetRichCurve())
				BackstepProbability = BackstepReactionDependency->Eval(Reaction);

			if (FMath::RandRange(0.f, 1.f) < BackstepProbability)
				RecommendedAction = ENpcDefensiveAction::StepOut;
		}	

		ActiveReactToAttackActor = ThreatActor;	
	}
	
	BlackboardComponent->SetValueAsEnum(NpcCombatDTR->NpcBlackboardDataAsset->DefenseActionBBKey.SelectedKeyName, (uint8)RecommendedAction);
}

void UNpcCombatLogicComponent::ReactToFeintedAttack(AActor* ThreatActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::ReactToFeintedAttack)
	
	const float PerceivedDistance = GetIntellectAffectedDistance((GetOwner()->GetActorLocation() - ThreatActor->GetActorLocation()).Size());
	const float PerceivedOwnerAttackRange = GetIntellectAffectedDistance(AttackRange);
	
	if (PerceivedDistance > PerceivedOwnerAttackRange)
	{
		UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("Didn't react to enemy feinted attack from %s because distance to attacker is longer than owner attacker range"), *ThreatActor->GetName());
		return;	
	}
	
	if (FMath::RandRange(0.f, 1.f) > Reaction * Intelligence)
	{
		UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("Didn't react to feint attack from %s because reaction check failed"), *ThreatActor->GetName());
		// AddIgnoredIncomingAttackFromThreat(ThreatActor, 0.25f);
		return; // if reaction check fails - "miss" an attacking threat
	}
	
	auto NpcCombatDTR = GetNpcDTR();

	// source of formula: i just felt this way at the moment of writing it
	const float ChanceToCounterAttack = (Reaction * Aggressiveness + (Stamina / OwnerAliveCreature->GetMaxStamina())) / 2.f;
	if (FMath::RandRange(0.f, 1.f) < ChanceToCounterAttack)
		BlackboardComponent->SetValueAsEnum(NpcCombatDTR->NpcBlackboardDataAsset->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::CounterAttack);
	else // in theory, this should abort block immediately
		BlackboardComponent->SetValueAsEnum(NpcCombatDTR->NpcBlackboardDataAsset->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::None);
}

void UNpcCombatLogicComponent::ReactToEnemyWhiffedAttack(AActor* ThreatActor)
{
	if (!ensure(OwnerAliveCreature->IsNpcActorAlive()))
		return;

	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::ReactToEnemyWhiffedAttack)
	
	const float PerceivedDistance = GetIntellectAffectedDistance((GetOwner()->GetActorLocation() - ThreatActor->GetActorLocation()).Size());
	const float PerceivedOwnerAttackRange = GetIntellectAffectedDistance(AttackRange);
	
	if (PerceivedDistance > PerceivedOwnerAttackRange)
	{
		UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("Didn't react to enemy whiffed attack from %s because distance to attacker is longer than owner attacker range"), *ThreatActor->GetName());
		return;	
	}
	
	auto NpcCombatDTR = GetNpcDTR();
	const float ChanceToCounterAttack = (Reaction * Aggressiveness + (Stamina / OwnerAliveCreature->GetMaxStamina())) / 2.f;
	if (FMath::RandRange(0.f, 1.f) < ChanceToCounterAttack)
		BlackboardComponent->SetValueAsEnum(NpcCombatDTR->NpcBlackboardDataAsset->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::CounterAttack);
}

void UNpcCombatLogicComponent::ReactToEnemyBlock(AActor* Actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::ReactToEnemyBlock)
	
	AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
	if (!ensure(AIController))
		return;

	if (Actor != AIController->GetFocusActor())
		return;
	
	auto BTComponent = AIController->FindComponentByClass<UEnhancedBehaviorTreeComponent>();
	FAIMessage AIMessage;
	AIMessage.Status = FAIMessage::Success;
	AIMessage.MessageName = AIGameplayTags::AI_BrainMessage_Attack_EnemyBlocking.GetTag().GetTagName();
	BTComponent->HandleMessageImmediately(AIMessage);
}

void UNpcCombatLogicComponent::ReactToEnemyChangeWeapon(AActor* Actor)
{
	// TODO update threat attack range
}

void UNpcCombatLogicComponent::ResetReactionToIncomingAttack()
{
	ActiveReactToAttackActor.Reset();
}

void UNpcCombatLogicComponent::OnAttributeAdded(UAbilitySystemComponent* ASC, const UAttributeSet* Attributes)
{
	if (Attributes->IsA<UNpcCombatAttributeSet>())
	{
		InitializeNpcCombatAttributeSet(ASC, Cast<const UNpcCombatAttributeSet>(Attributes));
	}
}

float UNpcCombatLogicComponent::GetCombatEvaluatorInterval() const
{
	return CombatEvaluatorInterval;
}

void UNpcCombatLogicComponent::OnNpcDeathStarted(AActor* OwningActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::OnNpcDeathStarted)
	
	ResetCurrentCombatTarget();
	
	for (auto& ActiveThreat : ActiveThreats)
	{
		if (auto Threat = Cast<IThreat>(ActiveThreat.Key))
		{
			Threat->OnEnemyPerformingAttackEvent.RemoveAll(this);
			Threat->OnEnemyFeintedAttackEvent.RemoveAll(this);
			Threat->OnEnemyAttackWhiffedEvent.RemoveAll(this);
			Threat->OnEnemyBlockEvent.RemoveAll(this);
			Threat->OnEnemyWeaponChangedEvent.RemoveAll(this);
		}
	}

	ActiveThreats.Reset();

	UnsubscribeFromDelegates();
}

TArray<APawn*> UNpcCombatLogicComponent::GetAllies(bool bIgnoreSquadLeader) const
{
	return GetWorld()->GetSubsystem<UNpcActivitySquadSubsystem>()->GetAllies(Cast<APawn>(GetOwner()), bIgnoreSquadLeader);
}

void UNpcCombatLogicComponent::SetActiveThreats(const FNpcActiveThreatsContainer& EvaluatedThreats)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::SetActiveThreats)
	
	TArray<AActor*> RemovedThreats;
	FNpcActiveThreatsContainer AddedThreats;

	for (auto& ActiveThreat : ActiveThreats)
	{
		if (!EvaluatedThreats.Contains(ActiveThreat.Key))
		{
			if (auto ForgottenThreat = Cast<IThreat>(ActiveThreat.Key))
			{
				ForgottenThreat->OnEnemyPerformingAttackEvent.RemoveAll(this);
				ForgottenThreat->OnEnemyFeintedAttackEvent.RemoveAll(this);
				ForgottenThreat->OnEnemyAttackWhiffedEvent.RemoveAll(this);
				ForgottenThreat->OnEnemyBlockEvent.RemoveAll(this);
				ForgottenThreat->OnEnemyWeaponChangedEvent.RemoveAll(this);
			}
			
			RemovedThreats.Add(ActiveThreat.Key.Get());
		}
	}

	for (const auto* RemovedThreat : RemovedThreats)
	{
		ActiveThreats.Remove(RemovedThreat);
	}
	
	for (const auto& Threat : EvaluatedThreats)
	{
		if (!ActiveThreats.Contains(Threat.Key))
		{
			FNpcThreatData AddedThreat = Threat.Value;
			if (auto ThreatInterface = Cast<IThreat>(Threat.Key.Get()))
			{
				AddedThreat.AttackRange = ThreatInterface->GetAttackRange();
				ThreatInterface->OnEnemyPerformingAttackEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToIncomingAttack);
				ThreatInterface->OnEnemyFeintedAttackEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToFeintedAttack);
				ThreatInterface->OnEnemyAttackWhiffedEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToEnemyWhiffedAttack);
				ThreatInterface->OnEnemyBlockEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToEnemyBlock);
				ThreatInterface->OnEnemyWeaponChangedEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToEnemyChangeWeapon);
			}

			ActiveThreats.Add(Threat.Key, AddedThreat);
		}
	}
	
	// ActiveThreats = EvaluatedThreats;
}

const FNpcActiveThreatsContainer& UNpcCombatLogicComponent::GetActiveThreats() const
{
	return ActiveThreats;
}

void UNpcCombatLogicComponent::SetCurrentCombatTarget(AActor* Target, const FNpcCombatPerceptionData& MobCombatPerceptionData, const FGameplayTag& BehaviorTypeTag)
{
	CurrentTargetData.ActiveTarget = Target;
	CurrentTargetData.ActiveBehaviorTypeTag = BehaviorTypeTag;
	CurrentTargetData.NpcCombatPerceptionData = MobCombatPerceptionData;
}

const FNpcActiveTargetData& UNpcCombatLogicComponent::GetCurrentCombatTarget() const
{
	return CurrentTargetData;
}

void UNpcCombatLogicComponent::ResetCurrentCombatTarget()
{
	CurrentTargetData.Reset();
}

const FNpcFeintParameters& UNpcCombatLogicComponent::GetAttackFeintParameters() const
{
	return GetNpcDTR()->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.FeintParameters;
}

void UNpcCombatLogicComponent::SetAttackRange(float NewValue)
{
	AttackRange = NewValue;
	if (auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset; BlackboardKeys->AttackRangeBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->AttackRangeBBKey.SelectedKeyName, NewValue);
	}
}

void UNpcCombatLogicComponent::SetSurroundRange(float NewValue)
{
	SurroundRange = NewValue;
	if (auto NpcBlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset; NpcBlackboardKeys->SurroundRangeBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->SurroundRangeBBKey.SelectedKeyName, NewValue);
	}
}

void UNpcCombatLogicComponent::SetAggression(float NewAggression)
{
	Aggressiveness = NewAggression;
	auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset;
	if (BlackboardKeys && BlackboardKeys->AggressivenessBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->AggressivenessBBKey.SelectedKeyName, Aggressiveness);
}

void UNpcCombatLogicComponent::SetIntelligence(float NewIntelligence)
{
	Intelligence = NewIntelligence;
	auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset;
	if (BlackboardKeys && BlackboardKeys->IntelligenceBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->IntelligenceBBKey.SelectedKeyName, Intelligence);
}

void UNpcCombatLogicComponent::SetReaction(float NewValue)
{
	Reaction = NewValue;
	auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset;
	if (BlackboardKeys && BlackboardKeys->ReactionBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->ReactionBBKey.SelectedKeyName, Reaction);
	}
}

void UNpcCombatLogicComponent::SetHealth(float NewValue)
{
	Health = NewValue;
	auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset;
	if (BlackboardKeys && BlackboardKeys->NormalizedHealthBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->NormalizedHealthBBKey.SelectedKeyName, Health / OwnerAliveCreature->GetMaxHealth());
	}
}

void UNpcCombatLogicComponent::SetStamina(float NewValue)
{
	Stamina = NewValue;
	auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset;
	if (BlackboardKeys && BlackboardKeys->NormalizedStaminaBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->NormalizedStaminaBBKey.SelectedKeyName, Stamina / OwnerAliveCreature->GetMaxStamina());
	}
}

void UNpcCombatLogicComponent::SetCombatEvaluatorInterval(float NewValue)
{
	CombatEvaluatorInterval = NewValue;
	auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset;
	if (BlackboardKeys && BlackboardKeys->CombatEvaluationIntervalBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(BlackboardKeys->CombatEvaluationIntervalBBKey.SelectedKeyName, CombatEvaluatorInterval);
	}
}

void UNpcCombatLogicComponent::OnAttackRangeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	SetAttackRange(OnAttributeChangeData.NewValue * AttackRangeScale);
}

void UNpcCombatLogicComponent::OnAggressivenessChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	SetAggression(OnAttributeChangeData.NewValue);
}

void UNpcCombatLogicComponent::OnHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	SetHealth(OnAttributeChangeData.NewValue);
	if (OnAttributeChangeData.OldValue > OnAttributeChangeData.NewValue && OnAttributeChangeData.GEModData != nullptr)
	{
		if (auto Instigator = OnAttributeChangeData.GEModData->EffectSpec.GetEffectContext().GetInstigator())
		{
			UAISense_Damage::ReportDamageEvent(this, GetOwner(), Instigator, OnAttributeChangeData.OldValue - OnAttributeChangeData.NewValue,
				Instigator->GetActorLocation(), GetOwner()->GetActorLocation());
		}
	}
}

void UNpcCombatLogicComponent::OnStaminaChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	SetStamina(OnAttributeChangeData.NewValue);
}

void UNpcCombatLogicComponent::OnIntellectChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	SetIntelligence(OnAttributeChangeData.NewValue);
}

void UNpcCombatLogicComponent::OnReactionChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	SetReaction(OnAttributeChangeData.NewValue);
}

FGameplayTag UNpcCombatLogicComponent::GetThreatLevel(float BestTargetThreat) const
{
	float ThreatDeviation = BestTargetThreat * (1.f - Intelligence);
	float IntellectAffectedPerceivedThreat = FMath::RandRange(BestTargetThreat - ThreatDeviation, BestTargetThreat + ThreatDeviation);
	const auto& ThreatLevels = GetNpcDTR()->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.ThreatLevels;
	for (int i = 0; i < ThreatLevels.Num(); i++)
	{
		if (ThreatLevels[i].ValueRange.Contains(IntellectAffectedPerceivedThreat))
			return ThreatLevels[i].Tag;
	}

	ensure(false);
	return AIGameplayTags::AI_Threat_Considerable;
}

float UNpcCombatLogicComponent::GetIntellectAffectedDistance(float BaseDistance) const
{
	if (DistanceIntelligenceDependcyFactorCachedIntelligence != Intelligence)
	{
		auto NpcDTR = GetNpcDTR();
		if (auto DistanceIntelligenceDependency = NpcDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.IntelligenceAttackRangeDeviationDependency.GetRichCurveConst())
		{
			DistanceIntelligenceDependcyFactorCachedIntelligence = Intelligence;
			DistanceIntelligenceDependcyFactor = DistanceIntelligenceDependency->Eval(Intelligence);
		}
	}

	float Deviation = BaseDistance * DistanceIntelligenceDependcyFactor;
	return FMath::RandRange(BaseDistance - Deviation, BaseDistance + Deviation);
}

void UNpcCombatLogicComponent::AddIgnoredIncomingAttackFromThreat(const AActor* Actor, float TimeToIgnore)
{
	float& IgnoreUntil = IgnoreIncomingAttackUntil.FindOrAdd(Actor);
	IgnoreUntil = GetWorld()->GetTimeSeconds() + TimeToIgnore;	
}

bool UNpcCombatLogicComponent::HasIgnoredIncomingAttackFromThreat(const AActor* Actor)
{
	if (auto ThreatIgnoredUntil = IgnoreIncomingAttackUntil.Find(Actor))
	{
		if (*ThreatIgnoredUntil > GetWorld()->GetTimeSeconds())
		{
			return true;
		}
		else
		{
			IgnoreIncomingAttackUntil.Remove(Actor);
		}
	}

	return false;
}

bool UNpcCombatLogicComponent::IsRetreating() const
{
	FGameplayTagContainer NpcTags = OwnerNPC->GetNpcOwnerTags();
	return NpcTags.HasTag(AIGameplayTags::AI_Behavior_Combat_Retreat);
}

bool UNpcCombatLogicComponent::IsSurrounding() const
{
	return ActiveAttackerRole != ENpcSquadRole::None;
}

void UNpcCombatLogicComponent::SetAttackerRole(ENpcSquadRole NpcAttackRole)
{
	ActiveAttackerRole = NpcAttackRole;
}

void UNpcCombatLogicComponent::UpdateBlackboardKeys()
{
	if (!bNpcComponentInitialized)
		return;
	
	SetAttackRange(AttackRange);
	SetSurroundRange(SurroundRange);
	SetAggression(Aggressiveness);
	SetIntelligence(Intelligence);
	SetReaction(Reaction);
	SetHealth(Health);
	SetCombatEvaluatorInterval(CombatEvaluatorInterval);
}

void UNpcCombatLogicComponent::InitializeNpcCombatLogic(AAIController& AIController)
{
	if (bNpcComponentInitialized)
		return;

	BlackboardComponent = AIController.GetBlackboardComponent();
	InitializeCombatData();
	bNpcComponentInitialized = true;
}

void UNpcCombatLogicComponent::UnsubscribeFromDelegates()
{
	auto ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC == nullptr)
		return;
	
	ASC->GetGameplayAttributeValueChangeDelegate(OwnerNPC->GetAttackRangeAttribute()).RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetAggressionAttribute()).RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetIntellectAttribute()).RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetReactionAttribute()).RemoveAll(this);
	if (OwnerAliveCreature != nullptr)
	{
		auto HealthAttribute = OwnerAliveCreature->GetHealthAttribute();
		ASC->GetGameplayAttributeValueChangeDelegate(HealthAttribute).RemoveAll(this);

		auto StaminaAttribute = OwnerAliveCreature->GetStaminaAttribute();
		ASC->GetGameplayAttributeValueChangeDelegate(StaminaAttribute).RemoveAll(this);
	}
}

void FNpcActiveTargetData::Reset()
{
	ActiveTarget.Reset();
	ActiveBehaviorTypeTag = FGameplayTag::EmptyTag;
	NpcCombatPerceptionData = {};
}