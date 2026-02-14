#include "Components/NpcCombatLogicComponent.h"

#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "GameplayEffectExtension.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/NpcInterfaceComponent.h"
#include "Components/Controller/EnhancedBehaviorTreeComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Data/NpcCombatParametersDataAsset.h"
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
	
	if (IsTemplate() || (GetOwner() && GetOwner()->IsTemplate()))
		return;

	if (const UWorld* World = GetWorld(); !World || !World->IsGameWorld())
		return;
	
	auto PawnLocal = Cast<APawn>(GetOwner());
	OwnerPawn = PawnLocal;
	
	OwnerNPC.SetObject(PawnLocal);
	OwnerNPC.SetInterface(Cast<INpc>(PawnLocal));
	
	OwnerAliveCreature.SetObject(PawnLocal);
	OwnerAliveCreature.SetInterface(Cast<INpcAliveCreature>(PawnLocal));
	OwnerAliveCreature->OnDeathStarted.AddUObject(this, &UNpcCombatLogicComponent::OnNpcDeathStarted);

	auto NpcCombatSettings = GetDefault<UNpcCombatSettings>();
	AttackRangeScale = NpcCombatSettings->AIAttackRangeScale;
	AttackRangeStepExtension = NpcCombatSettings->AttackRangeStepExtension;
	
	if (auto OwnerASCInterface = Cast<IAbilitySystemInterface>(PawnLocal))
		OwnerASC = OwnerASCInterface->GetAbilitySystemComponent();
	
	bool bNpcDataCached = false;
	auto NpcDTRH = OwnerNPC->GetNpcDataTableRowHandle();
	if (IsValid(NpcDTRH.DataTable) && !NpcDTRH.RowName.IsNone())
	{
		const auto* NpcDTR = NpcDTRH.GetRow<FNpcDTR>("");
		if (ensure(NpcDTR))
		{
			NpcCombatParameters = NpcDTR->NpcCombatParametersDataAsset;
			NpcBlackboardKeys = NpcDTR->NpcBlackboardDataAsset;
			bNpcDataCached = true;
		}
	}

	if (!bNpcDataCached)
	{
		int shit = 1;
		UE_LOG(LogARPGAI_CombatLogic, Error, TEXT("There was no NPC DTRH for UNpcCombatLogicComponent::InitializeComponent. He will be braindead"))
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

float UNpcCombatLogicComponent::GetBackdashProbabilityOnWhiff() const
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
	
	if (IsImmobilized())
		return;
	
	auto OwnerLocal = OwnerPawn.Get();
	
	if (!IsValid(ThreatActor))
	{
		UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Error, TEXT("UNpcCombatLogicComponent::ReactToIncomingAttack: threat actor is not valid"));
		FDebug::DumpStackTraceToLog(ELogVerbosity::Type::Warning);
		return;
	}
	
	UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Log, TEXT("Reacting to incoming attack from %s"), *ThreatActor->GetName());
	
	const auto* ThreatData = ActiveThreats.Find(ThreatActor);
	if (!ThreatData)
	{
		UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Error, TEXT("UNpcCombatLogicComponent::ReactToIncomingAttack: threat actor is not in NPCs active threats"));
		FDebug::DumpStackTraceToLog(ELogVerbosity::Type::Warning);
		return;
	}
	
#if WITH_EDITOR
	if (DebugOptions.Contains(TEXT("AlwaysParry")))
	{
		AttackingThreats.Add(ThreatActor);
		DefensiveActionCauser = ThreatActor;
		BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::Parry);
		return;
	}
#endif
	
	// @AK: FYI this formula is out of my ass. feel free to adjust if the parry chance seems to be too rare
	const float OtherAttacksReactionsScale = DefensiveActionCauser.IsValid() ? 0.5f : 1.f;
	const float ChanceToReact = FMath::Clamp(
		(Reaction + Intelligence * 0.75f - Aggressiveness * 0.25f) * NormalizedStamina * OtherAttacksReactionsScale,
		0.25f,
		0.9f);
	
	UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Log, TEXT("Chance to react = %.2f"), ChanceToReact);
	if (FMath::RandRange(0.f, 1.f) > ChanceToReact)
	{
		UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Log, TEXT("Didn't react to incoming attack from %s because reaction check failed"), *ThreatActor->GetName());
		// AddIgnoredIncomingAttackFromThreat(ThreatActor, 0.25f);
		return; // if reaction check fails - "miss" an attacking threat
	}

	FVector OwnerLocation = OwnerPawn->GetActorLocation();
	const float TrueDistance = (OwnerLocation - ThreatActor->GetActorLocation()).Size();
	const float PerceivedDistance = GetIntellectAffectedDistance(TrueDistance);
	UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Verbose, TEXT("True distance = %.2f; Perceived distance = %.2f"), TrueDistance, PerceivedDistance);
	const float PerceivedEnemyAttackRange = GetIntellectAffectedDistance(ThreatData->AttackRange + AttackRangeStepExtension);
	UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Verbose, TEXT("True attack range = %.2f; Perceived attack range = %.2f"), ThreatData->AttackRange + AttackRangeStepExtension, PerceivedEnemyAttackRange);
	
	if (PerceivedDistance > PerceivedEnemyAttackRange)
	{
		UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Log, TEXT("Didn't react to incoming attack from %s because perceived attack range is bigger than attackers range"), *ThreatActor->GetName());
		return;	
	}
	
	AttackingThreats.Add(ThreatActor);

	ENpcDefensiveAction RecommendedAction = ENpcDefensiveAction::Parry;
	int RelevantThreatsCount = 0;
	int EnemiesAttackingMe = 0;
	GetRelevantThreatsCounts(RelevantThreatsCount, EnemiesAttackingMe);
	if (EnemiesAttackingMe >= 2)
	{
		UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Verbose, TEXT("More than 1 threat is attacking me. Using dodge"));
		RecommendedAction = ENpcDefensiveAction::Dodge;
	}
	else
	{
		RecommendedAction = ENpcDefensiveAction::Parry;
		if (PerceivedDistance + NpcCombatParameters->NpcCombatEvaluationParameters.StepOutDistance > PerceivedEnemyAttackRange)
		{
			float BackdashProbability = 0.5f;
			if (const auto BackdashAggressionDependency = NpcCombatParameters->NpcCombatEvaluationParameters.BackstepAggressionProbabilityDependency.GetRichCurveConst())
				if (BackdashAggressionDependency->HasAnyData())
					BackdashProbability = BackdashAggressionDependency->Eval(Aggressiveness);

			UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Log, TEXT("Evaluating backdash probability: %.2f"), BackdashProbability);
			
			if (FMath::RandRange(0.f, 1.f) <= BackdashProbability)
			{
				UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Verbose, TEXT("Recommended action was parry but backdash chance occured: %.2f"), BackdashProbability);
				RecommendedAction = ENpcDefensiveAction::Backdash;
			}
		}	

		if (RecommendedAction == ENpcDefensiveAction::Parry)
		{
			// 13 Feb 2026 (aki): TODO add anxiety to calculation
			float RefuseParryUseDodgeProbability = (NormalizedStamina - 0.5f) * (1.2f - Aggressiveness * NormalizedHealth);
			RefuseParryUseDodgeProbability = FMath::Clamp(RefuseParryUseDodgeProbability, 0.05f, 0.9f);
			UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, VeryVerbose, TEXT("Unclamped refuse parry for dodge probability: %.2f"), RefuseParryUseDodgeProbability);
			if (FMath::RandRange(0.f, 1.f) <= RefuseParryUseDodgeProbability)
			{
				RecommendedAction = ENpcDefensiveAction::Dodge;
				UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Verbose, TEXT("Recommended action was parry but dodge chance occured: %.2f"), RefuseParryUseDodgeProbability);
			}
		}
		
		DefensiveActionCauser = ThreatActor;	
	}
	
	UE_VLOG(OwnerLocal, LogARPGAI_CombatLogic, Log, TEXT("Setting defense action from %s attack: %s"),
		*ThreatActor->GetName(), *StaticEnum<ENpcDefensiveAction>()->GetDisplayValueAsText(RecommendedAction).ToString());
	BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)RecommendedAction);
}

bool UNpcCombatLogicComponent::ShouldRetaliateAfterSuccessfulBlock(ENpcBlockResult BlockResult)
{
	if (BlockResult == ENpcBlockResult::None)
	{
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Warning, TEXT("Not reacting to successful block. Block result is none"));
		return false;
	}
	
	int RelevantEnemiesCount = 0;
	int EnemiesThatCanAttackMeCount = 0;
	GetRelevantThreatsCounts(RelevantEnemiesCount, EnemiesThatCanAttackMeCount);
	// -1 for parried/blocked enemy
	RelevantEnemiesCount--;
	EnemiesThatCanAttackMeCount--;
	
	const float ReactionScore = NpcCombatParameters->BlockRetaliationDesire_ReactionDependency.GetRichCurveConst()->HasAnyData() 
		? NpcCombatParameters->BlockRetaliationDesire_ReactionDependency.GetRichCurveConst()->Eval(Reaction)
		: Reaction;
	const float AggressivenessScale = NpcCombatParameters->BlockRetaliationDesireScale_AggressionDependency.GetRichCurveConst()->HasAnyData() 
		? NpcCombatParameters->BlockRetaliationDesireScale_AggressionDependency.GetRichCurveConst()->Eval(Aggressiveness)
		: Aggressiveness;
	const float StaminaScale = NpcCombatParameters->BlockRetaliationDesireScale_StaminaDependency.GetRichCurveConst()->HasAnyData() 
		? NpcCombatParameters->BlockRetaliationDesireScale_StaminaDependency.GetRichCurveConst()->Eval(NormalizedStamina)
		: NormalizedStamina;
	
	float RetaliationDesire = ReactionScore * AggressivenessScale * StaminaScale;
	if (NpcCombatParameters->BlockResultRetaliationDesireScales.Contains(BlockResult))
		RetaliationDesire *= NpcCombatParameters->BlockResultRetaliationDesireScales[BlockResult];
	
	const float CautiousnessReduction_EnemiesNearby = NpcCombatParameters->BlockRetaliationDesire_EnemiesAroundCountDependency.GetRichCurveConst()->HasAnyData() 
		? NpcCombatParameters->BlockRetaliationDesire_EnemiesAroundCountDependency.GetRichCurveConst()->Eval(RelevantEnemiesCount)
		: RelevantEnemiesCount / 10.f;
	const float CautiousnessReduction_EnemiesAttackingMe = NpcCombatParameters->BlockRetaliationDesire_EnemiesAttackingMeDependency.GetRichCurveConst()->HasAnyData() 
		? NpcCombatParameters->BlockRetaliationDesire_EnemiesAttackingMeDependency.GetRichCurveConst()->Eval(EnemiesThatCanAttackMeCount)
		: EnemiesThatCanAttackMeCount / 5.f;
	
	const float Score = RetaliationDesire - (CautiousnessReduction_EnemiesNearby + CautiousnessReduction_EnemiesAttackingMe) * Intelligence;
	const float Chance = FMath::RandRange(0.f, 1.f);
	const bool bRetaliate = Chance <= Score;

#if WITH_EDITOR
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("%s %s: [chance %.2f of %.2f]"),
		bRetaliate ? TEXT("Retaliate after successful block") : TEXT("Do not retaliate after successful"), 
		*StaticEnum<ENpcBlockResult>()->GetDisplayValueAsText(BlockResult).ToString(), Chance, Score);
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose,
		TEXT("Retaliation calculation:\nFormula: Retaliation desire (score) = reaction score * aggression scale * stamina scale * block result scale - cautiousness from attacking actors * intelligence"));
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Relevant enemies count = %d"), RelevantEnemiesCount);
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Dangerous attacking enemies = %d"), EnemiesThatCanAttackMeCount);
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Reaction score = %.2f (raw reaction = %.2f)"), ReactionScore, Reaction);
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Aggression scale = %.2f (raw aggression = %.2f)"), AggressivenessScale, Aggressiveness);
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Stamina scale = %.2f (raw stamina = %.2f)"), AggressivenessScale, NormalizedStamina);
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Cautiousness scale [enemies nearby] = %.2f"), CautiousnessReduction_EnemiesNearby);
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Cautiousness scale [enemies attacking me] = %.2f"), CautiousnessReduction_EnemiesAttackingMe);
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Intellect = %.2f"), Intelligence);
#endif
	
	return bRetaliate;
}

void UNpcCombatLogicComponent::GetRelevantThreatsCounts(int& RelevantEnemiesCount, int& EnemiesThatCanAttackMeCount, float RelevantDistanceScale, float Angle) const
{
	FVector OwnerLocation = OwnerPawn->GetActorLocation();
	float DotProductThreshold = FMath::Cos(FMath::DegreesToRadians(Angle));
	for (const auto& KnownThreat : ActiveThreats)
	{
		FVector ThreatToMe = OwnerLocation - KnownThreat.Key->GetActorLocation();
		float RelevantDistance = (KnownThreat.Value.AttackRange + AttackRange) * RelevantDistanceScale;
		float DistanceToThreatSq = ThreatToMe.SizeSquared();
		if (DistanceToThreatSq > RelevantDistance * RelevantDistance)
			continue;
		
		float DP = KnownThreat.Key->GetActorForwardVector() | ThreatToMe.GetSafeNormal();
		if (DP < DotProductThreshold)
			continue;
		
		RelevantEnemiesCount++;
		if (AttackingThreats.Contains(KnownThreat.Key))
			EnemiesThatCanAttackMeCount++;
	}
}

void UNpcCombatLogicComponent::ReactToThreatAttackCompleted(AActor* Actor)
{
	AttackingThreats.Remove(Actor);
	if (DefensiveActionCauser == Actor)
		ResetReactionToIncomingAttack();
}

void UNpcCombatLogicComponent::ReactToFeintedAttack(AActor* ThreatActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::ReactToFeintedAttack)
	
	if (IsImmobilized())
		return;
	
	AttackingThreats.Remove(ThreatActor);
	if (DefensiveActionCauser != ThreatActor)
		return;
	
	const float PerceivedDistance = GetIntellectAffectedDistance((GetOwner()->GetActorLocation() - ThreatActor->GetActorLocation()).Size());
	const float PerceivedOwnerAttackRange = GetIntellectAffectedDistance(AttackRange);
	
	if (PerceivedDistance > PerceivedOwnerAttackRange)
	{
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("Didn't react to enemy feinted attack from %s because distance to attacker is longer than owner attacker range"), *ThreatActor->GetName());
		return;	
	}
	
	if (FMath::RandRange(0.f, 1.f) > Reaction * Intelligence)
	{
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("Didn't react to feint attack from %s because reaction check failed"), *ThreatActor->GetName());
		// AddIgnoredIncomingAttackFromThreat(ThreatActor, 0.25f);
		return; // if reaction check fails - "miss" an attacking threat
	}
	
	// source of formula: i just felt this way at the moment of writing it
	const float ChanceToCounterAttack = (Reaction * Aggressiveness + (Stamina / OwnerAliveCreature->GetMaxStamina())) / 2.f;
	if (FMath::RandRange(0.f, 1.f) <= ChanceToCounterAttack)
		BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::CounterAttack);
	else // in theory, this should abort block immediately
		BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::None);
	
	ResetReactionToIncomingAttack();
}

void UNpcCombatLogicComponent::ReactToEnemyWhiffedAttack(AActor* ThreatActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::ReactToEnemyWhiffedAttack)

	if (IsImmobilized())
		return;
	
	AttackingThreats.Remove(ThreatActor);
	if (DefensiveActionCauser != ThreatActor)
		return;
	
	const float PerceivedDistance = GetIntellectAffectedDistance((GetOwner()->GetActorLocation() - ThreatActor->GetActorLocation()).Size());
	const float PerceivedOwnerAttackRange = GetIntellectAffectedDistance(AttackRange);
	
	if (PerceivedDistance > PerceivedOwnerAttackRange)
	{
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("Didn't react to enemy whiffed attack from %s because distance to attacker is longer than owner attacker range"), *ThreatActor->GetName());
		ResetReactionToIncomingAttack();
		return;	
	}
	
	const float ChanceToCounterAttack = (Reaction * Aggressiveness + (Stamina / OwnerAliveCreature->GetMaxStamina())) / 2.f;
	if (FMath::RandRange(0.f, 1.f) <= ChanceToCounterAttack)
		BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::CounterAttack);
	
	ResetReactionToIncomingAttack();
}

void UNpcCombatLogicComponent::ReactToEnemyBlock(AActor* Actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcCombatLogicComponent::ReactToEnemyBlock)

	if (IsImmobilized())
		return;
	
	if (Actor != PrimaryTargetData.ActiveTarget)
		return;
	
	auto BTComponent = OwnerPawn->GetController()->FindComponentByClass<UEnhancedBehaviorTreeComponent>();
	FAIMessage AIMessage;
	AIMessage.Status = FAIMessage::Success;
	AIMessage.MessageName = AIGameplayTags::AI_BrainMessage_Attack_EnemyBlocking.GetTag().GetTagName();
	BTComponent->HandleMessageImmediately(AIMessage);
}

void UNpcCombatLogicComponent::ReactToEnemyChangeWeapon(AActor* Actor)
{
	if (IsImmobilized())
		return;
	
	if (auto ThreatData = ActiveThreats.Find(Actor))
		if (auto Threat = Cast<IThreat>(Actor))
			ThreatData->AttackRange = Threat->GetAttackRange();
}

void UNpcCombatLogicComponent::ResetReactionToIncomingAttack()
{
#if WITH_EDITOR
	bool bWasValid = DefensiveActionCauser.IsValid();
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("Resetting reaction to incoming attack"));
	if (bWasValid)
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("Was reacting to attack from %s"), *DefensiveActionCauser->GetName());
	else 
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Warning, TEXT("By the way, active attacker is invalid"));
#endif		
	
	DefensiveActionCauser.Reset();
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
			Threat->OnThreatStartedAttackEvent.RemoveAll(this);
			Threat->OnThreatAttackCompletedEvent.RemoveAll(this);
			Threat->OnThreatFeintedAttackEvent.RemoveAll(this);
			Threat->OnThreatAttackWhiffedEvent.RemoveAll(this);
			Threat->OnThreatBlockEvent.RemoveAll(this);
			Threat->OnThreatWeaponChangedEvent.RemoveAll(this);
		}
	}

	ActiveThreats.Reset();
	AttackingThreats.Reset();
	UnsubscribeFromDelegates();
	bDead = true;
}

void UNpcCombatLogicComponent::SetEvaluatedTargetMoveDirection(ENpcTargetDistanceEvaluation NewTargetMoveDirectionEvaluation)
{
	TargetMoveDirectionEvaluation = NewTargetMoveDirectionEvaluation;
}

TArray<APawn*> UNpcCombatLogicComponent::GetAllies(bool bIgnoreSquadLeader) const
{
	return GetWorld()->GetSubsystem<UNpcSquadSubsystem>()->GetAllies(OwnerPawn.Get(), bIgnoreSquadLeader, true);
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
				ForgottenThreat->OnThreatStartedAttackEvent.RemoveAll(this);
				ForgottenThreat->OnThreatFeintedAttackEvent.RemoveAll(this);
				ForgottenThreat->OnThreatAttackWhiffedEvent.RemoveAll(this);
				ForgottenThreat->OnThreatBlockEvent.RemoveAll(this);
				ForgottenThreat->OnThreatWeaponChangedEvent.RemoveAll(this);
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
		if (auto* KnownThreat = ActiveThreats.Find(Threat.Key))
		{
			*KnownThreat = Threat.Value;
		}
		else
		{
			if (auto ThreatInterface = Cast<IThreat>(Threat.Key.Get()))
			{
				ThreatInterface->OnThreatStartedAttackEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToIncomingAttack);
				ThreatInterface->OnThreatAttackCompletedEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToThreatAttackCompleted);
				ThreatInterface->OnThreatFeintedAttackEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToFeintedAttack);
				ThreatInterface->OnThreatAttackWhiffedEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToEnemyWhiffedAttack);
				ThreatInterface->OnThreatBlockEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToEnemyBlock);
				ThreatInterface->OnThreatWeaponChangedEvent.AddUObject(this, &UNpcCombatLogicComponent::ReactToEnemyChangeWeapon);
			}

			ActiveThreats.Add(Threat.Key, Threat.Value);
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
	
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("UNpcCombatLogicComponent::SetCurrentCombatTarget: Setting new target data: %s [%s]"), 
		*Target->GetName(), *BehaviorTypeTag.ToString());
	
	PrimaryTargetData.ActiveTarget = Target;
	PrimaryTargetData.ActiveBehaviorTypeTag = BehaviorTypeTag;
	PrimaryTargetData.NpcCombatPerceptionData = {};
}

void UNpcCombatLogicComponent::ClearCurrentCombatTarget()
{
	if (PrimaryTargetData.ActiveTarget.IsValid())
	{
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("UNpcCombatLogicComponent::ClearCurrentCombatTarget: Clearing current combat target"));
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("UNpcCombatLogicComponent::ClearCurrentCombatTarget: Target was %s [%s]"), 
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
	NormalizedHealth = Health / OwnerAliveCreature->GetMaxHealth();
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->NormalizedHealthBBKey.SelectedKeyName != NAME_None)
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->NormalizedHealthBBKey.SelectedKeyName, NormalizedHealth);
}

void UNpcCombatLogicComponent::SetStamina(float NewValue)
{
	Stamina = NewValue;
	NormalizedStamina = Stamina / OwnerAliveCreature->GetMaxStamina();
	if (ensure(NpcBlackboardKeys) && NpcBlackboardKeys->NormalizedStaminaBBKey.SelectedKeyName != NAME_None)
	{
		BlackboardComponent->SetValueAsFloat(NpcBlackboardKeys->NormalizedStaminaBBKey.SelectedKeyName, NormalizedStamina);
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
	return ActiveAttackerRole == ENpcCombatRole::Attacker || ActiveAttackerRole == ENpcCombatRole::Surrounder;
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
	if (OwnerASC.IsValid())
	{
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
	}

	ResetTrackingEnemyAlive();
}


void UNpcCombatLogicComponent::TrackEnemyAlive(AActor* Actor)
{
	if (auto NewTrackedEnemyAlive = Cast<INpcAliveCreature>(Actor))
	{
		if (ensure(Actor != TrackedEnemyAlive))
		{
			UE_VLOG(this, LogARPGAI_CombatLogic, Verbose, TEXT("Tracking new enemy alive state: %s"), *Actor->GetName());
			
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
		UE_VLOG(this, LogARPGAI_CombatLogic, Verbose, TEXT("Not tracking enemy alive state anymore: %s"), *TrackedEnemyAlive->GetName());
		auto AliveCreature = Cast<INpcAliveCreature>(TrackedEnemyAlive.Get());
		AliveCreature->OnDeathStarted.RemoveAll(this);
		TrackedEnemyAlive.Reset();
	}
}

void UNpcCombatLogicComponent::ReactToReceivedHit(const FGameplayTag& HitTypeTag, AActor* HitCauser, float HealthDamage,
                                                  const FHitResult& HitResult)
{
	if (IsImmobilized() || HitCauser == nullptr || !NpcCombatParameters->RegularHitTypes.HasTag(HitTypeTag))
		return;
	
	const float NpcToCauserDP = OwnerPawn->GetActorForwardVector() | (HitCauser->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
	if (NpcToCauserDP < 0.65f)
		return;
	
	float BackdashProbability = NpcCombatParameters->BackdashChanceScaleOnHit * Reaction * NormalizedStamina;
	if (FMath::RandRange(0.f, 1.f) <= BackdashProbability)
	{
		UE_VLOG(OwnerPawn.Get(), LogARPGAI_CombatLogic, Verbose, TEXT("Received hit. Decided to request backdash (defense action). Probability = %.2f"), BackdashProbability);
		
#if WITH_EDITOR
		if (DebugOptions.Contains(TEXT("OnHitUseBackdashAsDefensiveAction")))
		{
			BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, static_cast<uint8>(ENpcDefensiveAction::Backdash));
			return;
		}
#endif

		auto NpcInterfaceComponent = OwnerPawn->FindComponentByClass<UNpcInterfaceComponent>();
		if (NpcInterfaceComponent != nullptr)
			NpcInterfaceComponent->Backdash();
	}
}

void UNpcCombatLogicComponent::Debug_RequestDodge()
{
	BlackboardComponent->SetValueAsEnum(NpcBlackboardKeys->DefenseActionBBKey.SelectedKeyName, (uint8)ENpcDefensiveAction::Dodge);
}

bool UNpcCombatLogicComponent::DecideWantToBaitAttack()
{
	auto WorldTime = GetWorld()->GetTimeSeconds();
	if (WorldTime < BaitAttackAvailableAtWorldTime)
	{
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("Can't bait attack. Cooldown is still active"));
		return false;
	}
	
	float AggressionScale = NpcCombatParameters->BaitAttackDesire_AggressionDependency.GetRichCurveConst()->HasAnyData() 
		? NpcCombatParameters->BaitAttackDesire_AggressionDependency.GetRichCurveConst()->Eval(Aggressiveness)
		: Aggressiveness;
	
	float Probability = NpcCombatParameters->BaitAttackProbabilityBase * Intelligence * NormalizedStamina - AggressionScale;
	UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Verbose, TEXT("Unclamped bait probability = %.2f"), Probability);
	Probability = FMath::Clamp(Probability, 0.1f, 0.9f);
	if (FMath::RandRange(0.f, 1.f) <= Probability)
	{
		UE_VLOG(GetOwner(), LogARPGAI_CombatLogic, Log, TEXT("Decided to bait attack"));
		BaitAttackAvailableAtWorldTime = WorldTime + NpcCombatParameters->BaitAttackCooldownAvg * FMath::RandRange(0.5f,  2.f);
		return true;
	}
	
	return false;
}

float UNpcCombatLogicComponent::GetBaitAttackDuration() const
{
	return NpcCombatParameters->BaitAttackDurationAvg * FMath::RandRange(0.5f,  2.f);
}

void UNpcCombatLogicComponent::OnEnemyDied(AActor* Actor)
{
	UE_VLOG(this, LogARPGAI_CombatLogic, Verbose, TEXT("Active enemy was killed"));
	
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
