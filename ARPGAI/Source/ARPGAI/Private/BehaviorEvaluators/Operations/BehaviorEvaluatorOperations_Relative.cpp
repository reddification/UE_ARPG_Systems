#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Relative.h"

#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Conditions.h"
#include "Components/EnemiesCoordinatorComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcMemoryComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcThreat.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/NpcSquadSubsystem.h"

float FBehaviorEvaluatorOperation_Relative_Base::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
                                                          const FCharacterShortTermMemory& CharacterPerceptionData, float CurrentScore) const
{
	bool bPassThrough = !bEnabled 
		|| !MaskPasses(CharacterPerceptionData.DetectionSource) 
		|| RequiredState.IsSet() && RequiredState.GetValue() != Context.EvaluatorState;
	
	if (bPassThrough)
		return CurrentScore;
	
	Context.AccumulatedScore = CurrentScore;
	float OpResult = 0.f;
	bool bOperationPointless = CurrentScore == 0.f 
		&& (OperationType == EBehaviorEvaluatorOperationType::Multiply 
			|| OperationType == EBehaviorEvaluatorOperationType::Divide 
			|| OperationType == EBehaviorEvaluatorOperationType::Power);
	
	if (!bOperationPointless)
		OpResult = EvaluateInternal(Context, Target, CharacterPerceptionData) * ConstScale;
	
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

FString FBehaviorEvaluatorOperation_Relative_Base::GetMaskDescription() const
{
	if (DetectionSourceMask.IsEmpty())
		return TEXT("");
	
	auto ReconstructedMask = GetReconstructedMask();
	FString Mask = FString::Printf(TEXT(" [Mask:%s"), bMaskCheckInverted ? TEXT(" NOT ") : TEXT(""));
	if ((ReconstructedMask & EDetectionSource::Ally) != EDetectionSource::None)
		Mask += TEXT(" Ally,");
	
	if ((ReconstructedMask & EDetectionSource::Assumption) != EDetectionSource::None)
		Mask += TEXT(" Assumption,");
	
	if ((ReconstructedMask & EDetectionSource::Audio) != EDetectionSource::None)
		Mask += TEXT(" Audio,");
	
	if ((ReconstructedMask & EDetectionSource::Damage) != EDetectionSource::None)
		Mask += TEXT(" Damage,");
	
	if ((ReconstructedMask & EDetectionSource::VisualMemory) != EDetectionSource::None)
		Mask += TEXT(" Visual Memory,");
	
	if ((ReconstructedMask & EDetectionSource::VisualActive) != EDetectionSource::None)
		Mask += TEXT(" Visual Active,");
		
	Mask.TrimCharInline(',', nullptr);
	Mask += "] ";
	
	return Mask;
}

EDetectionSource FBehaviorEvaluatorOperation_Relative_Base::GetReconstructedMask() const
{
	auto ReconstructedMask = EDetectionSource::None;
	for (const EDetectionSource DetectionSourceItem : DetectionSourceMask)
		ReconstructedMask |= DetectionSourceItem;
	
	return ReconstructedMask;
}

FString FBehaviorEvaluatorOperation_Relative_Base::GetShortDescription() const
{
	if (DetectionSourceMask.IsEmpty())
		return Super::GetShortDescription();
	
	auto Base = Super::GetShortDescription();
	FString Mask = GetMaskDescription();
	return FString::Printf(TEXT("%s%s"), *Base, *Mask);
}

FString FBehaviorEvaluatorOperation_Relative_Base::GenerateFormulaDescription(int Indentation) const
{
	if (DetectionSourceMask.IsEmpty())
		return Super::GenerateFormulaDescription(Indentation);
	
	auto Base = Super::GenerateFormulaDescription(Indentation);
	FString Mask = GetMaskDescription();
	return FString::Printf(TEXT("%s%s"), *Base, *Mask);
}

bool FBehaviorEvaluatorOperation_Relative_Base::MaskPasses(EDetectionSource DetectionSource) const
{
	if (DetectionSourceMask.IsEmpty())
		return true;

	auto ReconstructedMask = GetReconstructedMask();
	return (bStrictMask ? DetectionSource == ReconstructedMask : (DetectionSource & ReconstructedMask) != EDetectionSource::None) ^ bMaskCheckInverted; 
}

FString FBehaviorEvaluatorOperation_Aggregation::GenerateFormulaDescription(int Indentation) const
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

float FBehaviorEvaluatorOperation_Aggregation::EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target,
                                                                const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	float Result = 0.f;
	UE_CVLOG(Context.bLogEnabled, Context.Pawn.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Evaluating aggregation [%s]"), *GetShortDescription());
	for (const auto& OperationIS : Operations)
	{
		if (ensure(OperationIS.IsValid()))
		{
			const auto& Operation = OperationIS.Get<FBehaviorEvaluatorOperation_Relative_Base>();
			if (Operation.IsEnabled())
			{
				Result = Operation.Evaluate(Context, Target, CharacterPerceptionData, Result);
				UE_CVLOG(Context.bLogEnabled, Context.Pawn.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Local aggregation score after [%s] = %.2f"), *Operation.GetShortDescription(), Result);
			}
		}
	}
	
	return Result;
}

float FBehaviorEvaluatorOperation_IfElse::EvaluateInternal(const FRelativeOperationContext& Context,
                                                           const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	if (!Condition.IsValid())
		return FallbackValue;
	
	float Result = 0.f;
	if (Condition.Get().Evaluate(Context, Target, CharacterPerceptionData))
		return IfTrue.IsValid() ? IfTrue.Get().Evaluate(Context, Target, CharacterPerceptionData, Result) : FallbackValue;
	else 
		return IfFalse.IsValid() ? IfFalse.Get().Evaluate(Context, Target, CharacterPerceptionData, Result) : FallbackValue;
}

FString FBehaviorEvaluatorOperation_IfElse::GenerateFormulaDescription(int Indentation) const
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

FString FBehaviorEvaluatorOperation_IfElse::GetShortDescriptionInternal() const
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

float FBehaviorEvaluatorOperation_Distance::EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target,
                                                             const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(CharacterPerceptionData.Distance, 1.f);
}

float FBehaviorEvaluatorOperation_TimeSeen::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(CharacterPerceptionData.TimeSeen, 1.f);
}

float FBehaviorEvaluatorOperation_BehaviorDuration::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(Context.ActiveBehaviorDuration, 1.f);
}

float FBehaviorEvaluatorOperation_IndividualAccumulatedDamage::EvaluateInternal(const FRelativeOperationContext& Context,
                                                                                const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	const float Damage = bUseLongTermMemory ? CharacterPerceptionData.LongTermAccumulatedReceivedDamage : CharacterPerceptionData.ShortTermAccumulatedDamage;
	return DependencyCurve.GetRichCurveConst()->Eval(Damage / Context.MaxHealth, 1.f);
}

float FBehaviorEvaluatorOperation_CombatPerformance::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	const float X = CharacterPerceptionData.LongTermAccumulatedReceivedDamage / Context.MaxHealth;
	const float Y = CharacterPerceptionData.LongTermAccumulatedDealtDamage / CharacterPerceptionData.MaxHealth;
	return DependencyCurve.GetRichCurveConst()->Eval(Y - X, 1.f);
}

float FBehaviorEvaluatorOperation_AdvantageBase::EvaluateAdvantage(float Numerator, float Denumerator) const
{
	float Value = 0.f;
	if (!bInverted)
	{
		if (Denumerator != 0.f)
			Value = Numerator / Denumerator;
		else 
			Value = Numerator;
	}
	else 
	{
		if (Numerator != 0.f)
			Value = Denumerator / Numerator;
		else
			Value = Denumerator;
	}
	
	return DependencyCurve.GetRichCurveConst()->Eval(Value);
}

FString FBehaviorEvaluatorOperation_DamageOutputAdvantage::GenerateFormulaDescription(int Indentation) const
{
	return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("%s damage output advantage"), bInverted ? TEXT("NPC") : TEXT("Target"));
}

float FBehaviorEvaluatorOperation_DamageOutputAdvantage::EvaluateInternal(const FRelativeOperationContext& Context,
                                                                          const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	float MyDamageOutput = Cast<INpcThreat>(Context.Pawn)->GetDamageOutput_NpcThreat();
	return EvaluateAdvantage(MyDamageOutput, CharacterPerceptionData.DamageOutput);
}

FString FBehaviorEvaluatorOperation_ProtectionAdvantage::GenerateFormulaDescription(int Indentation) const
{
	return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("%s protection advantage"), bInverted ? TEXT("NPC") : TEXT("Target"));
}

float FBehaviorEvaluatorOperation_ProtectionAdvantage::EvaluateInternal(const FRelativeOperationContext& Context,
                                                                        const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	float MyProtection = Cast<INpcThreat>(Context.Pawn)->GetAverageProtection_NpcThreat();
	return EvaluateAdvantage(MyProtection, CharacterPerceptionData.Protection);
}

FString FBehaviorEvaluatorOperation_DamageOverProtectionAdvantage::GenerateFormulaDescription(int Indentation) const
{
	return Super::GenerateFormulaDescription(Indentation) 
		+ FString::Printf(TEXT("%s damage-over-protection advantage"), bMyOverTarget ? TEXT("NPC") : TEXT("Target"));
}

float FBehaviorEvaluatorOperation_DamageOverProtectionAdvantage::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	auto OwnerThreatInterface = Cast<INpcThreat>(Context.Pawn);
	return bMyOverTarget 
		? EvaluateAdvantage(OwnerThreatInterface->GetDamageOutput_NpcThreat(), CharacterPerceptionData.Protection) 
		: EvaluateAdvantage(CharacterPerceptionData.DamageOutput, OwnerThreatInterface->GetAverageProtection_NpcThreat());
}

float FBehaviorEvaluatorOperation_CountOfOtherAttackersOnTarget::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	auto EnemiesCoordinatorComponet = Target->FindComponentByClass<UEnemiesCoordinatorComponent>();
	if (EnemiesCoordinatorComponet == nullptr)
		return FallbackValue;
	
	int AttackersCount = EnemiesCoordinatorComponet->GetAttackersCount();
	return DependencyCurve.GetRichCurveConst()->Eval(AttackersCount);
}

float FBehaviorEvaluatorOperation_CountOfWitnessedMurderedAllies::EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	int CountOfKilledAllies = 0;
	if (auto OwnerMemoryComponent = GetNpcLongTermMemoryComponent(Context.Pawn.Get()); OwnerMemoryComponent != nullptr)
		CountOfKilledAllies = OwnerMemoryComponent->GetAlliesKilledByCount(Target, WithinGameTimeHours);
	
	return DependencyCurve.GetRichCurveConst()->Eval(CountOfKilledAllies);
}

float FBehaviorEvaluatorOperation_CountOfAlliesInCombat::EvaluateInternal(const FRelativeOperationContext& Context,
                                                                          const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	auto SquadSubsystem = UNpcSquadSubsystem::Get(Context.Pawn.Get());
	auto MyAllies = SquadSubsystem->GetAllies(Context.Pawn.Get(), true);
	int AlliesInCombatCount = 0;
	for (const auto* Ally: MyAllies)
	{
		auto AllyCombatLogicComponent = GetNpcCombatLogicComponent(Ally);
		if (AllyCombatLogicComponent->HasTarget(Target))
			AlliesInCombatCount++;
	}
	
	return DependencyCurve.GetRichCurveConst()->Eval(AlliesInCombatCount);
}

float FBehaviorEvaluatorOperation_DotProduct_OwnerFV_ToTarget::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(CharacterPerceptionData.DotProduct_OwnerFV_ToActor);
}

float FBehaviorEvaluatorOperation_DotProduct_OwnerFV_TargetFV::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(CharacterPerceptionData.ForwardVectorsDotProduct);
}

float FBehaviorEvaluatorOperation_DotProduct_TargetFV_ToOwner::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(CharacterPerceptionData.DotProduct_ActorFV_ToOwner);
}

float FBehaviorEvaluatorOperation_Scalar_TagBased::EvaluateInternal(const FRelativeOperationContext& Context,
                                                                    const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	return TagBasedParameters.Filter.Matches(CharacterPerceptionData.CharacterTags) ? TagBasedParameters.Value : FallbackValue;
}

float FBehaviorEvaluatorOperation_Scalar_AllyInCombat::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	auto SquadSubsystem = UNpcSquadSubsystem::Get(Context.Pawn.Get());
	auto MyAllies = SquadSubsystem->GetAllies(Context.Pawn.Get(), true);
	bool bAnyAllyInCombat = false;
	for (const auto* Ally: MyAllies)
	{
		auto AllyCombatLogicComponent = GetNpcCombatLogicComponent(Ally);
		if (AllyCombatLogicComponent->HasTarget(Target))
		{
			bAnyAllyInCombat = true;
			break;
		}
	}
	
	return bAnyAllyInCombat ? ConstantValue : FallbackValue;
}

FString FBehaviorEvaluatorOperation_Activation_VisualContactDuration::GenerateFormulaDescription(int Indentation) const
{
	return Super::Super::GenerateFormulaDescription(Indentation) 
		+ FString::Printf(TEXT("Activate: visual contact > %.2f ? %.2f | %.2f"), ActivationThreshold, ConstantValue, FallbackValue);
}

float FBehaviorEvaluatorOperation_Activation_VisualContactDuration::EvaluateInternal(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	return Activate(CharacterPerceptionData.HasVisualDetection() ? CharacterPerceptionData.TimeSeen : 0.f);
}

float FBehaviorEvaluatorOperation_Clamp::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterPerceptionData, float CurrentScore) const
{
	return DependencyCurve.GetRichCurveConst()->Eval(CurrentScore);
}

void FBehavorEvaluatorRelativeOperationsContainer::GenerateFormulaDescription()
{
	AutoGeneratedFormula.Reset();
	for (const auto& OperationIS : Operations)
		if (ensure(OperationIS.IsValid()))
			if (OperationIS.Get().IsEnabled())
				AutoGeneratedFormula += FString::Printf(TEXT("%s\n"), *OperationIS.Get().GenerateFormulaDescription(0));
}

float FBehaviorEvaluatorOperation_NonLinear_CountOfTargetAlliesInProximity::EvaluateInternal(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	auto TargetPawn = Cast<APawn>(Target);
	if (TargetPawn == nullptr)
		return FallbackValue;
	
	FVector TargetPawnLocation = TargetPawn->GetActorLocation();
	auto NpcSquadSubsystem = UNpcSquadSubsystem::Get(Target);
	auto AliveAllies = NpcSquadSubsystem->GetAllies(TargetPawn, true);
	int Count = 0;
	if (!AliveAllies.IsEmpty())
	{
		auto OwnerPerceptionComponent = Context.Pawn->GetController()->FindComponentByClass<UNpcPerceptionComponent>();
		const double DistanceThresholdSq = DistanceThreshold * DistanceThreshold;
		for (const auto& Ally : AliveAllies)
		{
			const auto* AllyPerceptionData = OwnerPerceptionComponent->GetShortTermCharactersMemory(Ally);
			if (AllyPerceptionData == nullptr || !AllyPerceptionData->HasVisualDetection())
				continue;
			
			const auto DistanceSq = (Ally->GetActorLocation() - TargetPawnLocation).SizeSquared();
			if (DistanceSq <= DistanceThresholdSq)
				Count++;
		}
	}
	
	return DependencyCurve.GetRichCurveConst()->Eval(Count);
}

float FBehaviorEvaluatorOperation_NonLinear_CountOfTargetAlliesOnWayToTarget::EvaluateInternal(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterPerceptionData) const
{
	auto TargetPawn = Cast<APawn>(Target);
	if (TargetPawn == nullptr)
		return FallbackValue;
	
	const FVector TargetPawnLocation = TargetPawn->GetActorLocation();
	const FVector MyLocation = Context.Pawn->GetActorLocation();
	auto NpcSquadSubsystem = UNpcSquadSubsystem::Get(Target);
	auto AliveAllies = NpcSquadSubsystem->GetAllies(TargetPawn, true);
	int Count = 0;
	const float DotProductThreshold = FMath::Cos(FMath::DegreesToRadians(AngleThreshold));
	if (!AliveAllies.IsEmpty())
	{
		auto OwnerPerceptionComponent = Context.Pawn->GetController()->FindComponentByClass<UNpcPerceptionComponent>();
		const double DistanceThresholdSq = DistanceThreshold * DistanceThreshold;
		for (const auto& Ally : AliveAllies)
		{
			const auto* AllyPerceptionData = OwnerPerceptionComponent->GetShortTermCharactersMemory(Ally);
			if (AllyPerceptionData == nullptr || !AllyPerceptionData->HasVisualDetection())
				continue;
			
			FVector AllyLocation = Ally->GetActorLocation();
			FVector FromNpcToTarget = TargetPawnLocation - MyLocation;
			FVector FromNpcToAlly = AllyLocation - MyLocation;
			
			float DP = FromNpcToTarget.GetSafeNormal() | FromNpcToAlly.GetSafeNormal();
			if (DP < DotProductThreshold)
				continue;
			
			float PathRatio = (FromNpcToTarget | FromNpcToAlly) / (FromNpcToTarget | FromNpcToTarget);
			if (PathRatio < 0.f || PathRatio > 1.f)
				continue;
			
			FVector ClosestPointToAllyOnPathToTarget = MyLocation + FromNpcToTarget * PathRatio;
			float DistanceSq = (ClosestPointToAllyOnPathToTarget - AllyLocation).SizeSquared();
			if (DistanceSq <= DistanceThresholdSq)
				Count++;
		}
	}
	
	return DependencyCurve.GetRichCurveConst()->Eval(Count);
}
