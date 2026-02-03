#pragma once
#include "GameplayTagContainer.h"

class UNpcAttitudesComponent;
class UNpcCombatLogicComponent;
class UNpcPerceptionComponent;
class UNpcComponent;
class AAIController;

namespace NpcCombatEvaluation
{
	enum EDetectionSource : uint8
	{
		None = 0,
		Damage = 1,
		Visual = Damage << 1,
		Ally = Visual << 1,
		Audio = Ally << 1
	};

	struct FNpcCombatPerceptionData
	{
		TMap<FGameplayTag, float> BehaviorUtilityScores;
		float Distance = 0.f;
		float PerceptionScore = 0.f;
		float ThreatScore = 0.f;
		bool bCharacter = false;
		FGameplayTagContainer Tags;
		
		EDetectionSource DetectionSource = EDetectionSource::None;
		
		void AddDetectionSource(EDetectionSource NewDetectionSource);
		bool HasDetectionSource(EDetectionSource TestDetectionSource) const;
	};

	struct FCombatEvaluationParameters
	{
		FCombatEvaluationParameters(const AAIController* AIController, UNpcComponent* InNpcComponent, UNpcAttitudesComponent* InNpcAttitudesComponent,
			UNpcCombatLogicComponent* InNpcCombatLogicComponent, const UNpcPerceptionComponent* InNpcPerceptionComponent,
			const APawn* InPawn, const float InMaxHealth)
			:	AIController(AIController),
				NpcComponent(InNpcComponent),
				NpcAttitudesComponent(InNpcAttitudesComponent),
				NpcCombatLogicComponent(InNpcCombatLogicComponent),
				PerceptionComponent(InNpcPerceptionComponent),
				Npc(InPawn),
				MaxHealth(InMaxHealth)
		{
		}

		const AAIController* AIController = nullptr;
		UNpcComponent* NpcComponent = nullptr;
		UNpcAttitudesComponent* NpcAttitudesComponent = nullptr;
		UNpcCombatLogicComponent* NpcCombatLogicComponent = nullptr;
		const UNpcPerceptionComponent* PerceptionComponent = nullptr;
		const APawn* Npc = nullptr;
		float MaxHealth = 0.f;
	};
	
	struct FCombatEvaluationResult
	{
		TMap<AActor*, FNpcCombatPerceptionData> DangerousActors;
		FVector OutInvestigationLocation = FVector::ZeroVector;
		float AccumulatedDamagePercent = 0.f;
	};
	
}
