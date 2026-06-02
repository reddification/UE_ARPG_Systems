#include "BehaviorEvaluators/BehaviorEvaluator_Anxiety.h"

#include "Components/Controller/NpcPerceptionComponent.h"
#include "Interfaces/NpcThreat.h"


TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Anxiety::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Anxiety>(*BTComponent, this);
}

FBehaviorEvaluator_Anxiety::FBehaviorEvaluator_Anxiety(UBehaviorTreeComponent& OwnerComp,
                                                       const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	AnxietyConfig = Cast<UBehaviorEvaluatorConfig_Anxiety>(Config);
}

void FBehaviorEvaluator_Anxiety::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(FBehaviorEvaluator_Anxiety::Update)
	
	float Anxiety = GetUtilityOffset();
	float Calmness = 0.f;
	
	auto OwnerThreat = Cast<INpcThreat>(Pawn.Get());
	const float MyStrength = OwnerThreat->GetDamageOutput_NpcThreat();
	const float MyProtection = OwnerThreat->GetAverageProtection_NpcThreat();

	const FRichCurve* DistanceToAllyDependency = AnxietyConfig->DistanceToAllyCalmnessScoreDependency.GetRichCurveConst();
	const FRichCurve* DistanceToEnemyDependency = AnxietyConfig->DistanceToEnemyAnxietyScoreDependency.GetRichCurveConst();
	const FRichCurve* StrengthAdvantageDependencyScale = AnxietyConfig->StrengthAdvantageScoreDependency.GetRichCurveConst();
	const FRichCurve* ProtectionAdvantageDependencyScale = AnxietyConfig->ProtectionAdvantageScoreDependency.GetRichCurveConst();
	
	const auto& CharactersPerception = PerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& CharacterPerception : CharactersPerception)
	{
		float TagsScore = 1.f;
		if (CharacterPerception.Value.bAlly)
		{
			for (const auto& AllyTagsFilterScale : AnxietyConfig->AllyTagsScoresScales)
				if (AllyTagsFilterScale.Filter.Matches(CharacterPerception.Value.CharacterTags))
					TagsScore += AllyTagsFilterScale.Value;
			
			float AllyStrength = CharacterPerception.Value.DamageOutput > 0 ? CharacterPerception.Value.DamageOutput : 1.f;
			float AllyProtection = CharacterPerception.Value.Protection > 0 ? CharacterPerception.Value.Protection : 1.f;
			float DistanceScore = DistanceToAllyDependency->Eval(CharacterPerception.Value.Distance);
			float StrengthAdvantageScale = StrengthAdvantageDependencyScale->Eval(AllyStrength / MyStrength);
			float ProtectionAdvantageScale = ProtectionAdvantageDependencyScale->Eval(AllyProtection / MyProtection);
			float IndividualScore = DistanceScore * (StrengthAdvantageScale + ProtectionAdvantageScale);
			Calmness += IndividualScore * TagsScore;
		}
		else if (CharacterPerception.Value.bHostile)
		{
			for (const auto& EnemyTagsFilterScale : AnxietyConfig->EnemiesTagsScoresScales)
				if (EnemyTagsFilterScale.Filter.Matches(CharacterPerception.Value.CharacterTags))
					TagsScore += EnemyTagsFilterScale.Value;
			
			float EnemyStrength = CharacterPerception.Value.DamageOutput > 0 ? CharacterPerception.Value.DamageOutput : 1.f;
			float EnemyProtection = CharacterPerception.Value.Protection > 0 ? CharacterPerception.Value.Protection : 1.f;
			float DistanceScore = DistanceToEnemyDependency->Eval(CharacterPerception.Value.Distance);
			float StrengthAdvantageScale = StrengthAdvantageDependencyScale->Eval(MyStrength / EnemyStrength);
			float ProtectionAdvantageScale = ProtectionAdvantageDependencyScale->Eval(MyProtection / EnemyProtection);
			float IndividualScore = DistanceScore * (StrengthAdvantageScale + ProtectionAdvantageScale);
			Anxiety += IndividualScore * TagsScore;
		}
	}
	
	InterpolateUtility(Anxiety - Calmness, DeltaTime);
}
