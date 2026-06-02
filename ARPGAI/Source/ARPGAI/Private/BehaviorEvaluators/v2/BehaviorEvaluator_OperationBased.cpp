#include "BehaviorEvaluators/v2/BehaviorEvaluator_OperationBased.h"

#include "AIController.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Data/NpcCombatTypes.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcActorTagsInterface.h"
#include "Interfaces/NpcAliveCreature.h"

void UBehaviorEvaluatorConfig_OperationBased::GenerateFormulasDescriptions()
{
	StatePressureOperationsContainer.GenerateFormulaDescription();
	EnemiesEvaluationParameters.IndividualOperationsContainer.GenerateFormulaDescription();
}

FBehaviorEvaluator_OperationBased::FBehaviorEvaluator_OperationBased(UBehaviorTreeComponent& OwnerComp,
                                                                     const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	OperationsConfig = Cast<UBehaviorEvaluatorConfig_OperationBased>(Config);
	Health = 0.f;
	MaxHealth = 0.f;
}

void FBehaviorEvaluator_OperationBased::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	auto AliveCreatureInterface = Cast<INpcAliveCreature>(Pawn.Get());
	Health = AliveCreatureInterface->GetHealth_NpcAliveCreature();
	MaxHealth = AliveCreatureInterface->GetMaxHealth_NpcAliveCreature();
	
	const float RegressionOffset = GetUtilityOffset();
	const float BehaviorUtility = Evaluate();
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Delta utility raw = %.2f + %.2f = %.2f"),
		RegressionOffset, BehaviorUtility, RegressionOffset + BehaviorUtility);
	InterpolateUtility(RegressionOffset + BehaviorUtility, DeltaTime);
}

float FBehaviorEvaluator_OperationBased::Evaluate()
{
	PreEvaluate();
	return 0.f;
}

FRelativeOperationContext FBehaviorEvaluator_OperationBased::GetRelativeOperationContext() const
{
	FRelativeOperationContext RelativeOperationContext;
	RelativeOperationContext.Pawn = Pawn;
	RelativeOperationContext.Health = Health;
	RelativeOperationContext.MaxHealth = MaxHealth;
	RelativeOperationContext.EvaluatorState = GetState();
	if (GetState() == EBehaviorEvaluatorState::Activated)
		RelativeOperationContext.ActiveBehaviorDuration = Pawn->GetWorld()->GetTimeSeconds() - BehaviorStartTime;
	
	return RelativeOperationContext;
}

FAggregationOperationContext FBehaviorEvaluator_OperationBased::GetAggregationOperationContext() const
{
	FAggregationOperationContext AggregationOperationContext;
	AggregationOperationContext.Pawn = Pawn;
	AggregationOperationContext.Health = Health;
	AggregationOperationContext.MaxHealth = MaxHealth;
	AggregationOperationContext.EvaluatorState = GetState();
	
	if (GetState() == EBehaviorEvaluatorState::Activated)
		AggregationOperationContext.ActiveBehaviorDuration = Pawn->GetWorld()->GetTimeSeconds() - BehaviorStartTime;
	
	if (auto TagsInterface = Cast<INpcActorTagsInterface>(Pawn))
		AggregationOperationContext.Tags = TagsInterface->GetTags_NPC();
	
	return AggregationOperationContext;
}

float FBehaviorEvaluator_OperationBased::CalculateStatePressure() const
{
	float RawStatePressure = 0.f;
	FAggregationOperationContext AggregationOperationData = GetAggregationOperationContext();
	for (const auto& OperationIS : OperationsConfig->StatePressureOperationsContainer.Operations)
	{
		if (!ensure(OperationIS.IsValid()))
			continue;
		
		const auto& Operation = OperationIS.Get();
		RawStatePressure = Operation.Evaluate(AggregationOperationData, PerceptionComponent.Get(), RawStatePressure);
		UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Aggregative state pressure after [%s] = %.2f"),
				*Operation.GetShortDescription(), RawStatePressure);
	}

	float SaturatedStateScore = OperationsConfig->StatePressureSaturationCurve.GetRichCurveConst()->Eval(RawStatePressure);
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Raw state pressure = %.2f"), RawStatePressure);
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Saturated state pressure = %.2f"), SaturatedStateScore);
	
	return SaturatedStateScore;
}

bool FBehaviorEvaluator_OperationBased::IsCharacterRelevant(const FCharacterPerceptionData& CharacterPerceptionData,
	const FEntityOperationEvaluationParameters& EvaluationParameters) const
{
	return EvaluationParameters.EntityRelevantQuery.IsEmpty() || EvaluationParameters.EntityRelevantQuery.Matches(CharacterPerceptionData.CharacterTags);
}

void FBehaviorEvaluator_OperationBased::ExecuteEntityOperations(AActor* Target, const FCharacterPerceptionData& CharacterPerception,
	const FRelativeOperationContext& RelativeOperationData, FActorScoresContainer& Container,
	const FEntityOperationEvaluationParameters& EvaluationParameters)
{
	if (!IsCharacterRelevant(CharacterPerception, EvaluationParameters))
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("[%s] %s is irrelevant"),
			*EvaluationParameters.DebugInfo, *CharacterPerception.CharacterId.ToString());
		return;
	}
	
	float IndividualScore = 0.f;
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Calculating individual score for %s"),
		*CharacterPerception.CharacterId.ToString());
	for (const auto& OperationIS : EvaluationParameters.IndividualOperationsContainer.Operations)
	{
		if (!ensure(OperationIS.IsValid()))
			continue;
				
		const auto& Operation = OperationIS.Get();
		if (!Operation.IsEnabled())
			continue;
		
		IndividualScore = Operation.Evaluate(RelativeOperationData, Target, CharacterPerception, IndividualScore);
		UE_VLOG(AIController.Get(), LogARPGAI_BE, VeryVerbose, TEXT("Score after [%s] = %.2f"),
			*Operation.GetShortDescription(), IndividualScore);
	}
			
	FBehaviorEvaluator_ActorScore ActorScore = FBehaviorEvaluator_ActorScore(Target, IndividualScore,
		CharacterPerception.DetectionSource, CharacterPerception.bAlive);
	Container.Add(ActorScore);
	OnIndividualScoreCalculated(Target, CharacterPerception, IndividualScore);
	
#if WITH_EDITOR
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Individual score for %s = %.2f"),
		*CharacterPerception.CharacterId.ToString(), IndividualScore);
	UE_VLOG_CAPSULE(AIController.Get(), LogARPGAI_BE, Verbose, Target->GetActorLocation() - FVector::UpVector * 90.f,
		90.f, 30.f, FQuat::Identity, FColorList::Turquoise, TEXT("Individual score = %.2f"), IndividualScore);
#endif
}

float FBehaviorEvaluator_OperationBased::GetEntitiesAggregatedScore(FActorScoresContainer& ActorScores,
	const FEntityOperationEvaluationParameters& EvaluationParameters) const
{
	if (ActorScores.IsEmpty())
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("No entities for [%s] -> entity evaluation = 0.f"), *EvaluationParameters.DebugInfo);
		return 0.f;
	}
	
	float RawResult = 0.f;
	const auto* OrderScaleCurve = EvaluationParameters.EntityOrderedScoreScale.GetRichCurveConst();
	ActorScores.Sort();
	for (int i = 0; i < ActorScores.Num(); ++i)
	{
		ActorScores[i].Score *= OrderScaleCurve->Eval(i);
		RawResult += ActorScores[i].Score;
	}

	const float SaturatedScore = EvaluationParameters.EntityPressureSaturationCurve.GetRichCurveConst()->Eval(RawResult);
	
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Raw score for [%s] = %.2f"), *EvaluationParameters.DebugInfo, RawResult);
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Saturated score for [%s] = %.2f"), *EvaluationParameters.DebugInfo, SaturatedScore);
	return SaturatedScore;
}
