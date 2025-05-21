// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeCombatComponent.h"
#include "Components/ActorComponent.h"
#include "PlayerHybridControlCombatComponent.generated.h"


// TODO
// unlike direct swing control method, this one should have 2 attack button (slashes/thrusts), one side control button (say gamepads right bumper/keyboards shift) and simplified direction
// so instead of 8 directions there will be only 3: up (for oberhauws), none (for muttlehaws) and low (for unterhauws/low attacks)
// fast double swing press + perfectly timed directional input = spin attack
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UPlayerHybridControlCombatComponent : public UMeleeCombatComponent
{
	GENERATED_BODY()
};
