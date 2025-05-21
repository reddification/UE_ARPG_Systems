// 

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ProtectionAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class COMBAT_API UProtectionAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CutDamageProtection;
	ATTRIBUTE_ACCESSORS(UProtectionAttributeSet, CutDamageProtection);

	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PunctureDamageProtection;
	ATTRIBUTE_ACCESSORS(UProtectionAttributeSet, PunctureDamageProtection);

	UPROPERTY(BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BludgeonDamageProtection;
	ATTRIBUTE_ACCESSORS(UProtectionAttributeSet, BludgeonDamageProtection);
};
