


#include "GAS/Executions/MobDamageExecutionCalculation.h"

#include "Data/AIGameplayTags.h"

struct FDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition HealthDef;
	FGameplayEffectAttributeCaptureDefinition ReceivedDamageScaleDef;
	FGameplayEffectAttributeCaptureDefinition PoiseDamageScaleDef;

	FDamageStatics()
	{
		// HealthDef = FGameplayEffectAttributeCaptureDefinition(ULyraHealthSet::GetHealthAttribute(), EGameplayEffectAttributeCaptureSource::Target,
		// 	false);
		// ReceivedDamageScaleDef = FGameplayEffectAttributeCaptureDefinition(ULyraHealthSet::GetReceivedDamageScaleAttribute(), EGameplayEffectAttributeCaptureSource::Target,
		// false);
		// PoiseDamageScaleDef = FGameplayEffectAttributeCaptureDefinition(ULyraCombatSet::GetPoiseDamageScaleAttribute(), EGameplayEffectAttributeCaptureSource::Target,
		// 	true);
	}
};

static FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}

UMobDamageExecutionCalculation::UMobDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(DamageStatics().HealthDef);

// #if WITH_EDITORONLY_DATA
// 	InvalidScopedModifierAttributes.Add(DamageStatics().HealthDef);
// #endif // #if WITH_EDITORONLY_DATA
}

void UMobDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	float CurrentHealth = 0.f;
	float CurrentPoiseDamageScale = 1.f;
	float ReceivedDamageScale = 1.f;
	
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().HealthDef, EvaluateParameters, CurrentHealth);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PoiseDamageScaleDef, EvaluateParameters, CurrentPoiseDamageScale);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ReceivedDamageScaleDef, EvaluateParameters, CurrentPoiseDamageScale);
	float AttackDamage = Spec.GetSetByCallerMagnitude(AIGameplayTags::AI_GameplayEffect_Attack_Damage_Health) * ReceivedDamageScale;
	float PoiseDamage = Spec.GetSetByCallerMagnitude(AIGameplayTags::AI_GameplayEffect_Attack_Damage_Poise) * CurrentPoiseDamageScale;
	
	// OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULyraHealthSet::GetHealthAttribute(), EGameplayModOp::Additive, -AttackDamage));
	// OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULyraCombatSet::GetPoiseAttribute(), EGameplayModOp::Additive, -PoiseDamage));
}
