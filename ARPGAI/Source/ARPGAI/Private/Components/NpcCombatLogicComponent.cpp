// 


#include "Components/NpcCombatLogicComponent.h"

#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "GameplayEffectExtension.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/Controller/EnhancedBehaviorTreeComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Threat.h"
#include "Perception/AISense_Damage.h"
#include "Settings/NpcCombatSettings.h"
#include "Subsystems/NpcSquadSubsystem.h"

UNpcCombatLogicComponent::UNpcCombatLogicComponent()
{
	bWantsInitializeComponent = true;
}

// Called when the game starts
void UNpcCombatLogicComponent::BeginPlay()
{
	Super::BeginPlay();
	if (auto AIController = Cast<AAIController>(OwnerPawn->GetController()); ensure(AIController))
		InitializeNpcCombatLogic(*AIController);
}

void UNpcCombatLogicComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromDelegates();
	Super::EndPlay(EndPlayReason);
}

void UNpcCombatLogicComponent::InitializeComponent()
{
	Super::InitializeComponent();
	auto PawnLocal = Cast<APawn>(GetOwner());
	OwnerPawn = PawnLocal;
	
	OwnerNPC.SetObject(PawnLocal);
	OwnerNPC.SetInterface(Cast<INpc>(PawnLocal));

	NpcDTRH = OwnerNPC->GetNpcDataTableRowHandle();
	
	OwnerAliveCreature.SetObject(PawnLocal);
	OwnerAliveCreature.SetInterface(Cast<INpcAliveCreature>(PawnLocal));
	OwnerAliveCreature->OnDeathStarted.AddUObject(this, &UNpcCombatLogicComponent::OnNpcDeathStarted);

	auto NpcCombatSettings = GetDefault<UNpcCombatSettings>();
	AttackRangeScale = NpcCombatSettings->AIAttackRangeScale;
	AttackRangeStepExtension = NpcCombatSettings->AttackRangeStepExtension;
	
	if (auto OwnerASCInterface = Cast<IAbilitySystemInterface>(PawnLocal))
		OwnerASC = OwnerASCInterface->GetAbilitySystemComponent();
	
	auto NpcDTR = GetNpcDTR();
	if (ensure(NpcDTR))
	{
		NpcCombatParameters = NpcDTR->NpcCombatParametersDataAsset;
		NpcBlackboardKeys = NpcDTR->NpcBlackboardDataAsset;
	}
}

void UNpcCombatLogicComponent::InitializeCombatData()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::InitializeCombatData)
	
	if (OwnerASC.IsValid())
	{
		if (auto NpcCombatAttributeSet = OwnerASC->GetSet<UNpcCombatAttributeSet>())
		{
			InitializeNpcCombatAttributeSet(NpcCombatAttributeSet);
		}

		auto HealthAttribute = OwnerAliveCreature->GetHealthAttribute();
		OwnerASC->GetGameplayAttributeValueChangeDelegate(HealthAttribute).AddUObject(this, &UNpcCombatLogicComponent::OnHealthChanged);
			SetHealth(OwnerASC->GetNumericAttribute(HealthAttribute));

		auto StaminaAttribute = OwnerAliveCreature->GetStaminaAttribute();
		OwnerASC->GetGameplayAttributeValueChangeDelegate(StaminaAttribute).AddUObject(this, &UNpcCombatLogicComponent::OnStaminaChanged);
			SetStamina(OwnerASC->GetNumericAttribute(StaminaAttribute));
	}
	
	if (const auto DistanceIntelligenceDependency = NpcCombatParameters->NpcCombatEvaluationParameters.IntelligenceAttackRangeDeviationDependency.GetRichCurveConst())
	{
		DistanceIntelligenceDependencyFactorCachedIntelligence = Intelligence;
		DistanceIntelligenceDependencyFactor = DistanceIntelligenceDependency->Eval(Intelligence);
	}
}

void UNpcCombatLogicComponent::InitializeNpcCombatAttributeSet(const UNpcCombatAttributeSet* NpcCombatAttributeSet)
{
	OwnerASC->GetGameplayAttributeValueChangeDelegate(OwnerNPC->GetAttackRangeAttribute())
	   .AddUObject(this, &UNpcCombatLogicComponent::OnAttackRangeChanged);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetAggressionAttribute())
	   .AddUObject(this, &UNpcCombatLogicComponent::OnAggressivenessChanged);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetIntellectAttribute())
	   .AddUObject(this, &UNpcCombatLogicComponent::OnIntellectChanged);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetReactionAttribute())
	   .AddUObject(this, &UNpcCombatLogicComponent::OnReactionChanged);
	
	SetAttackRange(OwnerNPC->GetAttackRange());
	SetSurroundRange(NpcCombatAttributeSet->GetSurroundRange());
	SetAggression(NpcCombatAttributeSet->GetAggression());
	SetIntelligence(NpcCombatAttributeSet->GetIntellect());
	SetReaction(NpcCombatAttributeSet->GetReaction());
}

float UNpcCombatLogicComponent::GetTauntProbabilityOnSuccessfulAttack() const
{
	IThreat* ActiveEnemy = OwnerNPC->GetActiveTarget();
	float EnemyHealth = ActiveEnemy->GetHealth();
	const auto& NpcCombatEvaluationParameters = NpcCombatParameters->NpcCombatEvaluationParameters;
	bool bShouldTaunt = ActiveEnemy->IsStaggered();
	if (!bShouldTaunt)
	{
		const float TrueDistanceBetweenEnemyAndNpc = (ActiveEnemy->GetThreatLocation() - GetOwner()->GetActorLocation()).Size();
		float PerceivedDistanceBetweenEnemyAndNpc = GetIntellectAffectedDistance(TrueDistanceBetweenEnemyAndNpc);
		bShouldTaunt = PerceivedDistanceBetweenEnemyAndNpc > GetIntellectAffectedDistance(AttackRange + ActiveEnemy->GetAttackRange()) * NpcCombatEvaluationParameters.SafeDistanceForTauntFactor;
	}

	bShouldTaunt &= Health / EnemyHealth > NpcCombatEvaluationParameters.NpcToEnemyHealthRatioToTauntStaggeredEnemy;
	bShouldTaunt |= FMath::RandRange(0.f, NpcCombatEvaluationParameters.ChanceToTauntEnemyInBadScenario) > Intelligence; 
	
	return bShouldTaunt ? NpcCombatParameters->NpcCombatEvaluationParameters.TauntChanceAfterAttack : 0.f;
	// TODO FMath::Min(StaminaDependency->Eval(Stamina), AggressionDependency->Eval(Aggression))
}

float UNpcCombatLogicComponent::GetBackstepProbabilityOnWhiff() const
{
	if (ensure(NpcCombatParameters))
		if (const auto Dependency = NpcCombatParameters->NpcCombatEvaluationParameters.BackstepStaminaProbabilityDependency.GetRichCurveConst())
			return Dependency->Eval(Stamina);

	return 0.f;
	// TODO consider dependency from stamina
	// TODO FMath::Min(StaminaDependency->Eval(Stamina), AggressionDependency->Eval(Aggression))
}

void UNpcCombatLogicComponent::SetEnemyThreatLevel(const FGameplayTag& InThreatLevelTag)
{
	ActiveThreatLevelTag = InThreatLevelTag;
	if (InThreatLevelTag.IsValid())
	{
		if (const float* MinAnxietyLevel = NpcCombatParameters->NpcCombatEvaluationParameters.PerceivedThreatToMinAnxiety.Find(InThreatLevelTag))
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

void UNpcCombatLogicComponent::ReactToIncomingAttack(AActor* ThreatActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::ReactToIncomingAttack)
	
	auto ThreatData = ActiveThreats.Find(ThreatActor);
	if (!ensure(ThreatData))
		return;
	
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
	const float PerceivedEnemyAttackRange = GetIntellectAffectedDistance(ThreatData->AttackRange + AttackRangeStepExtension);
	
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
		if (PerceivedDistance + NpcCombatParameters->NpcCombatEvaluationParameters.StepOutDistance > PerceivedEnemyAttackRange)
		{
			float BackstepProbability = 0.5f;
			if (const auto BackstepReactionDependency = NpcCombatParameters->NpcCombatEvaluationParameters.BackstepReactionProbabilityDependency.GetRichCurveConst())
				BackstepProbability = BackstepReactionDependency->Eval(Reaction);

			if (FMath::RandRange(0.f, 1.f) <= BackstepProbability)
				RecommendedAction = ENpcDefensiveAction::Backdash;
		}	

		ActiveReactToAttackActor = ThreatActor;	
	}
	
	UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("Setting defense action from %s attack: %s"),
		*ThreatActor->GetName(), *StaticEnum<ENpcDefensiveAction>()->GetDisplayValueAsText(RecommendedAction).ToString());
	BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)RecommendedAction);
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
	
	// source of formula: i just felt this way at the moment of writing it
	const float ChanceToCounterAttack = (Reaction * Aggressiveness + (Stamina / OwnerAliveCreature->GetMaxStamina())) / 2.f;
	if (FMath::RandRange(0.f, 1.f) <= ChanceToCounterAttack)
		BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::CounterAttack);
	else // in theory, this should abort block immediately
		BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::None);
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
	
	const float ChanceToCounterAttack = (Reaction * Aggressiveness + (Stamina / OwnerAliveCreature->GetMaxStamina())) / 2.f;
	if (FMath::RandRange(0.f, 1.f) <= ChanceToCounterAttack)
		BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::CounterAttack);
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
#if WITH_EDITOR
	bool bWasValid = ActiveReactToAttackActor.IsValid();
	UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("Resetting reaction to incoming attack"));
	if (bWasValid)
		UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("Was reacting to attack from %s"), *ActiveReactToAttackActor->GetName());
	else 
		UE_VLOG(GetOwner(), LogARPGAI, Warning, TEXT("By the way, active attacker is invalid"));
#endif		
	
	ActiveReactToAttackActor.Reset();
}

void UNpcCombatLogicComponent::OnAttributeAdded(UAbilitySystemComponent* ASC, const UAttributeSet* Attributes)
{
	if (Attributes->IsA<UNpcCombatAttributeSet>())
	{
		InitializeNpcCombatAttributeSet(Cast<const UNpcCombatAttributeSet>(Attributes));
	}
}

float UNpcCombatLogicComponent::GetCombatEvaluatorInterval() const
{
	return CombatEvaluatorInterval;
}

void UNpcCombatLogicComponent::OnNpcDeathStarted(AActor* OwningActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::OnNpcDeathStarted)
	
	ClearCurrentCombatTarget();
	
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

void UNpcCombatLogicComponent::SetEvaluatedTargetMoveDirection(ENpcTargetDistanceEvaluation NewTargetMoveDirectionEvaluation)
{
	TargetMoveDirectionEvaluation = NewTargetMoveDirectionEvaluation;
}

TArray<APawn*> UNpcCombatLogicComponent::GetAllies(bool bIgnoreSquadLeader) const
{
	return GetWorld()->GetSubsystem<UNpcSquadSubsystem>()->GetAllies(Cast<APawn>(GetOwner()), bIgnoreSquadLeader, true);
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

void UNpcCombatLogicComponent::SetCurrentCombatTarget(AActor* Target, const FGameplayTag& BehaviorTypeTag,
                                                      const FNpcCombatPerceptionData& MobCombatPerceptionData)
{
	SetCurrentCombatTarget(Target, BehaviorTypeTag);
	PrimaryTargetData.NpcCombatPerceptionData = MobCombatPerceptionData;
}

void UNpcCombatLogicComponent::SetCurrentCombatTarget(AActor* Target, const FGameplayTag& BehaviorTypeTag)
{
	if (!ensure(IsValid(Target)))
		return;

	if (Target == PrimaryTargetData.ActiveTarget)
		return;
	
	if (PrimaryTargetData.ActiveTarget.IsValid())
		ClearCurrentCombatTarget();
	
	UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("UNpcCombatLogicComponent::SetCurrentCombatTarget: Setting new target data: %s [%s]"), 
		*Target->GetName(), *BehaviorTypeTag.ToString());
	
	PrimaryTargetData.ActiveTarget = Target;
	PrimaryTargetData.ActiveBehaviorTypeTag = BehaviorTypeTag;
	PrimaryTargetData.NpcCombatPerceptionData = {};
}

void UNpcCombatLogicComponent::ClearCurrentCombatTarget()
{
	UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("UNpcCombatLogicComponent::ClearCurrentCombatTarget: Clearing current combat target"));
	if (PrimaryTargetData.ActiveTarget.IsValid())
	{
		UE_VLOG(GetOwner(), LogARPGAI, Log, TEXT("UNpcCombatLogicComponent::ClearCurrentCombatTarget: Target was %s [%s]"), 
			*PrimaryTargetData.ActiveTarget->GetName(), *PrimaryTargetData.ActiveBehaviorTypeTag.ToString());
	}
	
	PrimaryTargetData.Reset();
}

const FNpcFeintParameters& UNpcCombatLogicComponent::GetAttackFeintParameters() const
{
	return NpcCombatParameters->NpcCombatEvaluationParameters.FeintParameters;
}

void UNpcCombatLogicComponent::UpdateAttackRangeInBlackboard()
{
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->AttackRangeBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->AttackRangeBBKey.SelectedKeyName, AttackRange);
}

void UNpcCombatLogicComponent::SetAttackRange(float NewValue)
{
	AttackRange = NewValue * AttackRangeScale + AttackRangeStepExtension;
	UpdateAttackRangeInBlackboard();
}

void UNpcCombatLogicComponent::SetSurroundRange(float NewValue)
{
	SurroundRange = NewValue;
	if (NpcBlackboardKeys->SurroundRangeBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->SurroundRangeBBKey.SelectedKeyName, NewValue);
}

void UNpcCombatLogicComponent::SetAggression(float NewAggression)
{
	Aggressiveness = NewAggression;
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->AggressivenessBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->AggressivenessBBKey.SelectedKeyName, Aggressiveness);
}

void UNpcCombatLogicComponent::SetIntelligence(float NewIntelligence)
{
	Intelligence = NewIntelligence;
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->IntelligenceBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->IntelligenceBBKey.SelectedKeyName, Intelligence);
}

void UNpcCombatLogicComponent::SetReaction(float NewValue)
{
	Reaction = NewValue;
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->ReactionBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->ReactionBBKey.SelectedKeyName, Reaction);
}

void UNpcCombatLogicComponent::SetHealth(float NewValue)
{
	Health = NewValue;
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->NormalizedHealthBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->NormalizedHealthBBKey.SelectedKeyName,
			Health / OwnerAliveCreature->GetMaxHealth());
	}
}

void UNpcCombatLogicComponent::SetStamina(float NewValue)
{
	Stamina = NewValue;
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->NormalizedStaminaBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->NormalizedStaminaBBKey.SelectedKeyName,
			Stamina / OwnerAliveCreature->GetMaxStamina());
	}
}

void UNpcCombatLogicComponent::SetCombatEvaluatorInterval(float NewValue)
{
	CombatEvaluatorInterval = NewValue;
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->CombatEvaluationIntervalBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->CombatEvaluationIntervalBBKey.SelectedKeyName, CombatEvaluatorInterval);
}

void UNpcCombatLogicComponent::OnAttackRangeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	SetAttackRange(OnAttributeChangeData.NewValue);
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
	const auto& ThreatLevels = NpcCombatParameters->NpcCombatEvaluationParameters.ThreatLevels;
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
	if (DistanceIntelligenceDependencyFactorCachedIntelligence != Intelligence)
	{
		if (auto DistanceIntelligenceDependency = NpcCombatParameters->NpcCombatEvaluationParameters.IntelligenceAttackRangeDeviationDependency.GetRichCurveConst())
		{
			DistanceIntelligenceDependencyFactorCachedIntelligence = Intelligence;
			DistanceIntelligenceDependencyFactor = DistanceIntelligenceDependency->Eval(Intelligence);
		}
	}

	float Deviation = BaseDistance * DistanceIntelligenceDependencyFactor;
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
	return ActiveAttackerRole != ENpcCombatRole::None;
}

void UNpcCombatLogicComponent::SetCombatRole(ENpcCombatRole NpcAttackRole)
{
	ActiveAttackerRole = NpcAttackRole;
}

void UNpcCombatLogicComponent::UpdateBlackboardKeys()
{
	if (!bNpcComponentInitialized)
		return;

	UpdateAttackRangeInBlackboard();
	SetSurroundRange(SurroundRange);
	SetAggression(Aggressiveness);
	SetIntelligence(Intelligence);
	SetReaction(Reaction);
	SetHealth(Health);
	SetCombatEvaluatorInterval(CombatEvaluatorInterval);
}

void UNpcCombatLogicComponent::SetDistanceToTarget(float NewDistance)
{
	DistanceToTarget = NewDistance;
	if (OwnerASC.IsValid())
	{
		const UNpcCombatAttributeSet* CombatAttributeSet = Cast<UNpcCombatAttributeSet>(OwnerASC->GetAttributeSet(UNpcCombatAttributeSet::StaticClass()));
		if (ensure(CombatAttributeSet != nullptr))
			OwnerASC->SetNumericAttributeBase(CombatAttributeSet->GetDistanceToTargetAttribute(), NewDistance);
		
		// 	CombatAttributeSet->SetDistanceToTarget(NewDistance);
			// OwnerASC->ApplyModToAttribute()
	}
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
	if (!OwnerASC.IsValid())
		return;
	
	OwnerASC->GetGameplayAttributeValueChangeDelegate(OwnerNPC->GetAttackRangeAttribute()).RemoveAll(this);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetAggressionAttribute()).RemoveAll(this);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetIntellectAttribute()).RemoveAll(this);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UNpcCombatAttributeSet::GetReactionAttribute()).RemoveAll(this);
	if (OwnerAliveCreature != nullptr)
	{
		auto HealthAttribute = OwnerAliveCreature->GetHealthAttribute();
		OwnerASC->GetGameplayAttributeValueChangeDelegate(HealthAttribute).RemoveAll(this);

		auto StaminaAttribute = OwnerAliveCreature->GetStaminaAttribute();
		OwnerASC->GetGameplayAttributeValueChangeDelegate(StaminaAttribute).RemoveAll(this);
	}

	ResetTrackingEnemyAlive();
}


void UNpcCombatLogicComponent::TrackEnemyAlive(AActor* Actor)
{
	if (auto NewTrackedEnemyAlive = Cast<INpcAliveCreature>(Actor))
	{
		if (ensure(Actor != TrackedEnemyAlive))
		{
			UE_VLOG(this, LogARPGAI, Verbose, TEXT("Tracking new enemy alive state: %s"), *Actor->GetName());
			
			NewTrackedEnemyAlive->OnDeathStarted.AddUObject(this, &UNpcCombatLogicComponent::OnEnemyDied);
			if (TrackedEnemyAlive.IsValid())
			{
				auto NpcAliveCreature = Cast<INpcAliveCreature>(TrackedEnemyAlive.Get());
				NpcAliveCreature->OnDeathStarted.RemoveAll(this);
			}
			
			TrackedEnemyAlive = Actor;
		}
	}
}

void UNpcCombatLogicComponent::ResetTrackingEnemyAlive()
{
	if (TrackedEnemyAlive.IsValid())
	{
		UE_VLOG(this, LogARPGAI, Verbose, TEXT("Not tracking enemy alive state anymore: %s"), *TrackedEnemyAlive->GetOwner()->GetName());
		auto AliveCreature = Cast<INpcAliveCreature>(TrackedEnemyAlive.Get());
		AliveCreature->OnDeathStarted.RemoveAll(this);
		TrackedEnemyAlive.Reset();
	}
}

void UNpcCombatLogicComponent::Debug_RequestDodge()
{
	BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::Dodge);
}

void UNpcCombatLogicComponent::OnEnemyDied(AActor* Actor)
{
	UE_VLOG(this, LogARPGAI, Verbose, TEXT("Active enemy was killed"));
	
	if (!TrackedEnemyAlive.IsValid())
		return;

	auto AliveCreature = Cast<INpcAliveCreature>(Actor);
	AliveCreature->OnDeathStarted.RemoveAll(this);
	if (Actor != TrackedEnemyAlive.Get())
		return;
	
	TrackedEnemyAlive.Reset();

	if (ActiveThreats.Num() > 1)
		return;

	if (!NpcBlackboardKeys->IsAllEnemiesKilledBBKey.SelectedKeyName.IsNone())
		BlackboardComponent->SetValueAsBool(NpcBlackboardKeys->IsAllEnemiesKilledBBKey.SelectedKeyName, true);

	auto SquadSubsystem = UNpcSquadSubsystem::Get(this);
	auto Allies = SquadSubsystem->GetAllies(Cast<APawn>(GetOwner()), false, true);
	// report to allies
	for (const auto& SquadMember : Allies)
	{
		if (SquadMember == GetOwner())
			continue;

		SquadMember->FindComponentByClass<UNpcCombatLogicComponent>()->ReceiveReportEnemyDied(Actor);
	}
}

void UNpcCombatLogicComponent::ReceiveReportEnemyDied(AActor* KilledActor)
{
	if (!TrackedEnemyAlive.IsValid() || ActiveThreats.Num() > 1)
		return;

	if (!NpcBlackboardKeys->IsAllEnemiesKilledBBKey.SelectedKeyName.IsNone())
		BlackboardComponent->SetValueAsBool(NpcBlackboardKeys->IsAllEnemiesKilledBBKey.SelectedKeyName, true);
}

void FNpcActiveTargetData::Reset()
{
	ActiveTarget.Reset();
	ActiveBehaviorTypeTag = FGameplayTag::EmptyTag;
	NpcCombatPerceptionData = {};
}
