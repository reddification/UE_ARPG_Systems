#include "BehaviorEvaluators/v2/BehaviorEvaluator_Anxiety_v2.h"
#include "Components/Controller/NpcPerceptionComponent.h"

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Anxiety_v2::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Anxiety_v2>(*BTComponent, this);
}

void UBehaviorEvaluatorConfig_Anxiety_v2::GenerateFormulasDescriptions()
{
	Super::GenerateFormulasDescriptions();
	AlliesEvaluationParameters.IndividualOperationsContainer.GenerateFormulaDescription();
}

FBehaviorEvaluator_Anxiety_v2::FBehaviorEvaluator_Anxiety_v2(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config) 
	: Super(OwnerComp, Config)
{
	AnxietyConfig = Cast<UBehaviorEvaluatorConfig_Anxiety_v2>(Config);
}

float FBehaviorEvaluator_Anxiety_v2::Evaluate()
{
	// 9 Apr 2026 (aki): deliberately NOT calling super, 
	// because whilst anxiety inherits a lot of useful things from FBehaviorEvaluator_OperationBased, it needs to override some of them completely
	// Super::Evaluate();
	FRelativeOperationContext RelativeOperationData_Enemies = GetRelativeOperationContext();
	FRelativeOperationContext RelativeOperationData_Allies = GetRelativeOperationContext();
	FActorScoresContainer Enemies; 
	FActorScoresContainer Allies;
	
	const auto& CharactersPerception = PerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& CharacterPerception : CharactersPerception)
	{
		if (CharacterPerception.Value.bAlly)
		{
			ExecuteEntityOperations(CharacterPerception.Key.Get(), CharacterPerception.Value,
				RelativeOperationData_Allies, Allies, AnxietyConfig->AlliesEvaluationParameters);
		}
		else if (CharacterPerception.Value.bHostile)
		{
			ExecuteEntityOperations(CharacterPerception.Key.Get(), CharacterPerception.Value,
				RelativeOperationData_Enemies, Enemies, OperationsConfig->EnemiesEvaluationParameters);
		}
	}
	
	float AnxietyFromSounds = 0.f;
	auto ActorHeardSounds = PerceptionComponent->GetHeardSounds();
	if (!ActorHeardSounds.IsEmpty())
	{
		FVector FV = Pawn->GetActorForwardVector();
		FVector PawnLocation = Pawn->GetActorLocation();
		for (const auto& HeardSoundList : ActorHeardSounds)
		{
			for (const auto& HeardSound : HeardSoundList.Value)
			{
				const float* HeardSoundAnxietyBasePtr = AnxietyConfig->AnxietyInducingSounds.Find(HeardSound.SoundTag);
				if (HeardSoundAnxietyBasePtr)
				{
					const float DistanceScore = AnxietyConfig->AnxietySoundToDistance.GetRichCurveConst()->Eval(HeardSound.Distance);
					const float DP = FV | (HeardSound.Location - PawnLocation).GetSafeNormal();
					const float DotProductScore = AnxietyConfig->SoundDotProductToAnxietyScale.GetRichCurveConst()->Eval(DP); 
					AnxietyFromSounds += *HeardSoundAnxietyBasePtr * DistanceScore * DotProductScore;
				}
			}
		}
	}
	
	if (AnxietyFromSounds != 0.f)
		AnxietyFromSounds = AnxietyConfig->AnxietyFromSoundsSaturationCurve.GetRichCurveConst()->Eval(AnxietyFromSounds);
	
	float Anxiety = GetEntitiesAggregatedScore(Enemies, OperationsConfig->EnemiesEvaluationParameters);
	float Confidence = GetEntitiesAggregatedScore(Allies, AnxietyConfig->AlliesEvaluationParameters);
	float StatePressure = CalculateStatePressure();
	
	return StatePressure * (Anxiety + AnxietyFromSounds) - Confidence;
}