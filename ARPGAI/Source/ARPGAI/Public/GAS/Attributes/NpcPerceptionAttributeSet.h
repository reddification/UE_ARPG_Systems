// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NpcPerceptionAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ARPGAI_API UNpcPerceptionAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UNpcPerceptionAttributeSet();
	ATTRIBUTE_ACCESSORS(UNpcPerceptionAttributeSet, SightRadius);
	ATTRIBUTE_ACCESSORS(UNpcPerceptionAttributeSet, SightHalfAngle);
	ATTRIBUTE_ACCESSORS(UNpcPerceptionAttributeSet, HearingRadius);

protected:
	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SightRadius;

	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SightHalfAngle;
	
	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData HearingRadius;
	
};
