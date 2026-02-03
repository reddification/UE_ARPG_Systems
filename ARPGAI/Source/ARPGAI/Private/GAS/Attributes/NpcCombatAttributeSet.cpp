// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Attributes/NpcCombatAttributeSet.h"

#include "GameplayEffectExtension.h"

UNpcCombatAttributeSet::UNpcCombatAttributeSet()
{
	SurroundRange = 500.f;
	Intellect = 1.f;
	Aggression = 10.f;
	AggressionRestoreRate = 0.05f;
	MaxAggression = 10.f;
	Anxiety = 0.f;
	MaxAnxiety = 1.f;
	DistanceToTarget = 0.f;
}

bool UNpcCombatAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	bool bSuper = Super::PreGameplayEffectExecute(Data);
	if (!bSuper) return false;

	return true;
}

void UNpcCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetAggressionAttribute())
	{
		SetAggression(FMath::Clamp(GetAggression(), 0.f, GetMaxAggression()));
	}
}

void UNpcCombatAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UNpcCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UNpcCombatAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	if (Attribute == GetMaxAggressionAttribute())
	{
		// Make sure current health is not greater than the new max health.
		if (GetAggression() > NewValue)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			if (ensure(ASC))
			{
				ASC->ApplyModToAttribute(GetAggressionAttribute(), EGameplayModOp::Override, NewValue);
			}
		}
	}
}

void UNpcCombatAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetAggressionAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAggression());
	}
	else if (Attribute == GetMaxAggressionAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}
