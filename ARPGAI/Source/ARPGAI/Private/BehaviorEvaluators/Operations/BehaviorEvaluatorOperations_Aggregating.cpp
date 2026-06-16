#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Aggregating.h"

#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcThreat.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/NpcSquadSubsystem.h"

float FBehaviorEvaluatorOperation_Aggregating_Base::Evaluate(const FAggregationOperationContext& Context, 
	const UNpcPerceptionComponent* NpcPerceptionComponent, float CurrentScore) const
{
	bool bPassThrough = !bEnabled 
		|| RequiredState.IsSet() && RequiredState.GetValue() != Context.EvaluatorState;
	
	if (bPassThrough)
		return CurrentScore;
	
	// Прости меня Господи
	Context.AccumulatedScore = CurrentScore;
	float OpResult = 0.f;
	bool bOperationPointless = CurrentScore == 0.f 
		&& (OperationType == EBehaviorEvaluatorOperationType::Multiply 
			|| OperationType == EBehaviorEvaluatorOperationType::Divide 
			|| OperationType == EBehaviorEvaluatorOperationType::Power);
	
	if (!bOperationPointless)
		OpResult = EvaluateInternal(Context, NpcPerceptionComponent) * ConstScale;
	
	if (bCacheResult && CachedResultName.IsValid())		
		Context.CachedVariables.Add(CachedResultName, OpResult);
		
	switch (OperationType)
	{
		case EBehaviorEvaluatorOperationType::Add:
			return CurrentScore + OpResult; 
		case EBehaviorEvaluatorOperationType::Multiply:
			return CurrentScore * OpResult; 
		case EBehaviorEvaluatorOperationType::Divide:
			return OpResult != 0.f ? CurrentScore / OpResult : CurrentScore;
		case EBehaviorEvaluatorOperationType::Subtract:
			return CurrentScore - OpResult;
		case EBehaviorEvaluatorOperationType::Power:
			return FMath::Pow(CurrentScore, OpResult);
		default:
			return CurrentScore;
	}
}

float FBehaviorEvaluatorOperation_TotalAccumulatedDamageFromEnemies::EvaluateInternal(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	float TotalDamage = 0.f;
	const auto& PerceptionData = NpcPerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& Perception : PerceptionData)
		if (Perception.Value.bHostile && Perception.Value.bAlive)
			TotalDamage += Perception.Value.ShortTermAccumulatedDamage;
	
	return DependencyCurve.GetRichCurveConst()->Eval(TotalDamage / Context.MaxHealth, 1.f);
}

float FBehaviorEvaluatorOperation_NormalizedHealth::EvaluateInternal(const FAggregationOperationContext& Context,
                                                                     const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(Context.Health / Context.MaxHealth, 1.f);
}

float FBehaviorEvaluatorOperation_Aggregating_BehaviorDuration::EvaluateInternal(
	const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(Context.ActiveBehaviorDuration, 1.f);
}

float FBehaviorEvaluatorOperation_OwnerTags::EvaluateInternal(const FAggregationOperationContext& Context,
                                                              const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return TagQuery.Matches(Context.Tags) ? ValuePositive : FallbackValue;
}

float FBehaviorEvaluatorOperation_CountOfCharacters::EvaluateInternal(const FAggregationOperationContext& Context,
                                                                      const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	int Count = 0;
	const auto& CharactersPerceptions = NpcPerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& CharacterData : CharactersPerceptions)
	{
		if (CharacterData.Value.Distance > MaxDistance)
			continue;
		
		if (DetectionSourceMask != EDetectionSource::None)
		{
			auto FilteredDetectionSource = CharacterData.Value.DetectionSource & DetectionSourceMask;
			const bool bDrop = bDetectionSourceMaskStrict 
				? FilteredDetectionSource != DetectionSourceMask 
				: FilteredDetectionSource == EDetectionSource::None;
			
			if (bDrop)
				continue;
		}
		
#if WITH_EDITOR
		ensure(!AttitudeTagOptions.IsEmpty() || !ActorFilter.IsEmpty());  // what the fuck do you want then?
#endif
		
		if (!AttitudeTagOptions.IsEmpty() && !CharacterData.Value.Attitude.MatchesAny(AttitudeTagOptions))
			continue;
		
		if (!ActorFilter.IsEmpty() && !ActorFilter.Matches(CharacterData.Value.CharacterTags))
			continue;
		
		Count++;
	}		
	
	return DependencyCurve.GetRichCurveConst()->Eval(Count);
}

float FBehaviorEvaluatorOperation_CountOfAliveAllies::EvaluateInternal(const FAggregationOperationContext& Context,
                                                                       const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	int Count = 0;
	float DistanceThresholdSq = MaxDistance * MaxDistance;
	const FVector PawnLocation = Context.Pawn->GetActorLocation();
	auto Allies = UNpcSquadSubsystem::Get(Context.Pawn.Get())->GetAllies(Context.Pawn.Get(), true);
	for (const auto* Ally : Allies)
		if ((Ally->GetActorLocation() - PawnLocation).SizeSquared() <= DistanceThresholdSq)
			Count++;
	
	return DependencyCurve.GetRichCurveConst()->Eval(Count);
}

float FBehaviorEvaluatorOperation_PowerBalance::EvaluateInternal(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	auto OwnerThreatInterface = Cast<INpcThreat>(Context.Pawn.Get());
	int AlliesCount = 1;
	float CombinedAlliesPower = OwnerThreatInterface->GetDamageOutput_NpcThreat();
	float CombinedAlliesProtection = OwnerThreatInterface->GetAverageProtection_NpcThreat();
	float AlliesHealthPool = Context.Health;
	int EnemiesCount = 0;
	float CombinedEnemiesPower = 0.f;
	float CombinedEnemiesProtection = 0.f;
	float EnemiesHealthPool = 0.f;
	
	const auto& AllCharactersSTM = NpcPerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& CharacterData : AllCharactersSTM)
	{
		if (!CharacterData.Value.bAlive || CharacterData.Value.Distance > MaxDistance)
			continue;
		
		if (CharacterData.Value.bAlly)
		{
			AlliesCount++;
			CombinedAlliesPower += CharacterData.Value.DamageOutput;
			CombinedAlliesProtection += CharacterData.Value.Protection;
			AlliesHealthPool += CharacterData.Value.Health;
		}
		else if (CharacterData.Value.bHostile)
		{
			EnemiesCount++;
			CombinedEnemiesPower += CharacterData.Value.DamageOutput;
			CombinedEnemiesProtection += CharacterData.Value.Protection;
			EnemiesHealthPool += CharacterData.Value.Health;
		}
	}
	
	// The final damage calculation equation is: final damage = raw damage * e^(-protection / ProtectionEffectivenessScale)
	bool bUseSqrt = FMath::IsNearlyEqual(0.5f, HealthFactor, 0.0001f);
	const float AdjustedAlliesHealthPool = bUseSqrt ? FMath::Sqrt(AlliesHealthPool) : FMath::Pow(AlliesHealthPool, HealthFactor);
	const float EnemiesProtectionScaledPower = CombinedEnemiesPower * FMath::Exp(-CombinedAlliesProtection / ProtectionEffectivenessScale);
	const float TimeToKill_EnemiesOverAllies = AdjustedAlliesHealthPool / FMath::Max(1.f, EnemiesProtectionScaledPower);
	
	const float AdjustedEnemiesHealthPool = bUseSqrt ? FMath::Sqrt(EnemiesHealthPool) : FMath::Pow(EnemiesHealthPool, HealthFactor);
	const float AlliesProtectionScaledPower = CombinedAlliesPower * FMath::Exp(-CombinedEnemiesProtection / ProtectionEffectivenessScale); 
	const float TimeToKill_AlliesOverEnenmies = AdjustedEnemiesHealthPool / FMath::Max(1.f, AlliesProtectionScaledPower);
	
	const float PowerAdvantage = TimeToKill_EnemiesOverAllies / FMath::Max(TimeToKill_AlliesOverEnenmies, KINDA_SMALL_NUMBER);
	// const float PowerAdvantage = ((CombinedAlliesPower * AverageAlliesNormalizedHealth) / (CombinedEnemiesProtection * AverageEnemiesNormalizedHealth)) 
	// 	- ((CombinedEnemiesPower * AverageEnemiesNormalizedHealth) / (CombinedAlliesProtection * AverageAlliesNormalizedHealth));

	return DependencyCurve.GetRichCurveConst()->Eval(PowerAdvantage);
}

FString FBehaviorEvaluatorOperation_Aggregative_Aggregation::GenerateFormulaDescription(int Indentation) const
{
	FString Base = Super::GenerateFormulaDescription(Indentation);
	FString AggregatedFormula = TEXT("");

	FString IndentationStr = FString::ChrN(Indentation, '\t');
	for (const auto& OperationIS : Operations)
		if (ensure(OperationIS.IsValid()))
		{
			if (OperationIS.Get().IsEnabled())
				AggregatedFormula += TEXT("\n") + OperationIS.Get().GenerateFormulaDescription(Indentation + 1);
		}
		else
			AggregatedFormula += TEXT("\nInvalid IS");
	
	return FString::Printf(TEXT("%s Aggregation\n%s(%s\n%s)"), *Base, *IndentationStr, *AggregatedFormula, *IndentationStr);
}

float FBehaviorEvaluatorOperation_Aggregative_Aggregation::EvaluateInternal(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	float Result = 0.f;
	UE_CVLOG(Context.bLogEnabled, Context.Pawn.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Evaluating aggregation [%s]"), *GetShortDescription());
	for (const auto& OperationIS : Operations)
	{
		if (ensure(OperationIS.IsValid()))
		{
			const auto& Operation = OperationIS.Get<FBehaviorEvaluatorOperation_Aggregating_Base>();
			if (Operation.IsEnabled())
			{
				Result = Operation.Evaluate(Context, NpcPerceptionComponent, Result);
				UE_CVLOG(Context.bLogEnabled, Context.Pawn.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Local aggregation score after [%s] = %.2f"), *Operation.GetShortDescription(), Result);
			}
		}
	}
	
	return Result;
}

FString FBehaviorEvaluatorOperation_Aggregating_IfElse::GenerateFormulaDescription(int Indentation) const
{
	FString ConditionalString = Condition.IsValid() ? Condition.Get().ToString(Indentation) : TEXT("Condition IS is invalid");
	FString IfTrueFormulaString = IfTrue.IsValid() ? IfTrue.Get().GenerateFormulaDescription(Indentation + 2) : TEXT("\"If True\" IS is invalid");
	FString IfFalseFormulaString = IfFalse.IsValid() ? IfFalse.Get().GenerateFormulaDescription(Indentation + 2) : TEXT("\"If False\" IS is invalid ");
	FString IndentationStr = FString::ChrN(Indentation, '\t');
	return FString::Printf(TEXT("%s\n%sIF\n%s\n%s\n%sELSE\n%s"),
		*Super::GenerateFormulaDescription(Indentation), *IndentationStr, *ConditionalString,
		*IfTrueFormulaString,
		*IndentationStr,
		*IfFalseFormulaString);
}

float FBehaviorEvaluatorOperation_Aggregating_IfElse::EvaluateInternal(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	if (!Condition.IsValid())
		return FallbackValue;
	
	float Result = 0.f;
	if (Condition.Get().Evaluate(Context, NpcPerceptionComponent))
		return IfTrue.IsValid() ? IfTrue.Get().Evaluate(Context, NpcPerceptionComponent, Result) : FallbackValue;
	else 
		return IfFalse.IsValid() ? IfFalse.Get().Evaluate(Context, NpcPerceptionComponent, Result) : FallbackValue;
}

FString FBehaviorEvaluatorOperation_Aggregating_IfElse::GetShortDescriptionInternal() const
{
	if (!Condition.IsValid())
		return TEXT("Error: condition IS is not set");
	
	if (!IfTrue.IsValid())
		return TEXT("Error: If true IS is not set");
	
	if (!IfFalse.IsValid())
		return TEXT("Error: If false IS is not set");
		
	return FString::Printf(TEXT("If %s\n\t%s\nElse\n\t%s"), 
		*Condition.Get().ToString(0), *IfTrue.Get().GetShortDescription(), *IfFalse.Get().GetShortDescription());
}

void FBehavorEvaluatorAggregatingOperationsContainer::GenerateFormulaDescription()
{
	AutoGeneratedFormula.Reset();
	for (const auto& OperationIS : Operations)
		if (ensure(OperationIS.IsValid()))
			if (OperationIS.Get().IsEnabled())
				AutoGeneratedFormula += FString::Printf(TEXT("%s\n"), *OperationIS.Get().GenerateFormulaDescription(0));
}
