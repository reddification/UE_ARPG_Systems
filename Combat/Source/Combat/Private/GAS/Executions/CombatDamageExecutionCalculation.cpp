// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Executions/CombatDamageExecutionCalculation.h"

#include "AbilitySystemComponent.h"
#include "Data/MeleeCombatSettings.h"
#include "GAS/Data/GameplayEffectContext_MeleeDamage.h"
#include "Interfaces/ICombatant.h"

UCombatDamageExecutionCalculation::UCombatDamageExecutionCalculation()
{
}

void UCombatDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	// FGameplayEffectContext* TypedContext = Spec.GetContext().Get();
	// check(TypedContext);
	//
	// const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	// const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	//
	// FAggregatorEvaluateParameters EvaluateParameters;
	// EvaluateParameters.SourceTags = SourceTags;
	// EvaluateParameters.TargetTags = TargetTags;
	//
	// const AActor* EffectCauser = TypedContext->GetEffectCauser();
	// const FHitResult* HitActorResult = TypedContext->GetHitResult();
	// const ICombatant* Attacker = Cast<ICombatant>(EffectCauser);
	//
	// // Clamping is done when damage is converted to -health
	// const float DamageDone = FMath::Clamp(BaseDamage * DistanceAttenuation * PhysicalMaterialAttenuation * DamageInteractionAllowedMultiplier, 0.0f);
	//
	// if (DamageDone > 0.0f)
	// {
	// 	// Apply a damage modifier, this gets turned into - health on the target
	// 	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULyraHealthSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageDone));
	// }
}
