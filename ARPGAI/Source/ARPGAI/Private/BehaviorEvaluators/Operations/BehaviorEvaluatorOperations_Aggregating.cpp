#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Aggregating.h"

#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
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
	UE_VLOG(Context.Pawn.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Evaluating aggregation [%s]"), *GetShortDescription());
	for (const auto& OperationIS : Operations)
	{
		if (ensure(OperationIS.IsValid()))
		{
			const auto& Operation = OperationIS.Get<FBehaviorEvaluatorOperation_Aggregating_Base>();
			if (Operation.IsEnabled())
			{
				Result = Operation.Evaluate(Context, NpcPerceptionComponent, Result);
				UE_VLOG(Context.Pawn.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Local aggregation score after [%s] = %.2f"), *Operation.GetShortDescription(), Result);
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
