// Copyright Pixagon Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcInfoWidgetComponent.h"
#include "NpcCombatSettings.generated.h"

UCLASS(Config=Game, defaultconfig, DisplayName="NPC Combat")
class ARPGAI_API UNpcCombatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	uint8 DefaultPerceptionTeamId = 255;
	
	// Max amount of surrounding attackers 
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1), Category="Surround")
	int MaxAttackers = 3;

	// Max amount of surrounding taunters
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1), Category="Surround")
	int MaxSurrounders = 10;

	// Min space between attackers on inner attacker circle. Used for generating EQS items
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(UIMin = 50.f, ClampMin = 50.f), Category="Surround")
	float AttackersEqsSpaceBetween = 80.f;

	// Min space between taunters on outer taunter  circle. Used for generating EQS items
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(UIMin = 100.f, ClampMin = 100.f), Category="Surround")
	float TauntersEqsSpaceBetween = 200.f;
	
	/** Used inside mob's EQS test, determines maximum angle between player's forward vector and vector from player to query item. Anything higher than that is filterd out*/
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, UIMax = 90.f, ClampMin = 0.f, ClampMax = 90.f), Category="Surround")
	float SurroundDeltaAngle = 30.f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	FName RagdollRootBone = "pelvis";

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float RagdollImpulse = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.1f, ClampMin = 0.1f))
	float AIAttackRangeScale = 1.333;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.1f, ClampMin = 0.1f))
	float AttackRangeStepExtension = 80.f;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Perception", meta=(Categories="AI.Noise"))
	FGameplayTagContainer DangerousSounds;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	float SurroundAvoidancePathfindingScore = 50.f;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<class UNpcStateWidget> NpcInfoWidgetClass;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="UI")
	float NpcInfoWidgetVisibilityUpdateInterval = 1.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="UI")
	float NpcInfoWidgetConsiderableDistanceToHostilePlayer = 850.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="UI")
	float NpcInfoWidgetConsiderableDistanceToNonHostilePlayer = 250.f;
	
	// Unused
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="UI")
	TSoftObjectPtr<UCurveFloat> WidgetScaleDistanceToPlayerDependency;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="UI")
	FName NpcInfoWidgetSocketName = FName("Socket_NpcInfoWidget");
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="UI", meta=(UIMin = -1.f, ClampMin = -1.f, ClampMax = 1.f, UIMax = 1.f))
	float MinPlayerToNpcDotProductToShowWidget = -0.5f;
};
