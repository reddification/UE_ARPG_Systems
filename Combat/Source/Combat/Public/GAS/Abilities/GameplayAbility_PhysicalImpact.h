// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_HitReact.h"
#include "GameplayAbility_PhysicalImpact.generated.h"

/**
 *  This is basically hit react (i.e. it only activates an additive montage)
 *  but the purpose of having a separate ability is to have a different set of cancel abilities / block abilities
 *  e.g. HitReact cancels active attack and block but physical impact doesn't. But they both apply additive montage to give a feel of receiving impact
 */
UCLASS()
class COMBAT_API UGameplayAbility_PhysicalImpact : public UGameplayAbility_HitReact
{
	GENERATED_BODY()

public:
	UGameplayAbility_PhysicalImpact();
};
