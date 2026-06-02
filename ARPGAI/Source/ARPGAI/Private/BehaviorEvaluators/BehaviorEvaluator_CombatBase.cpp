#include "BehaviorEvaluators/BehaviorEvaluator_CombatBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"

UBehaviorEvaluatorConfig_CombatBase::UBehaviorEvaluatorConfig_CombatBase()
{
	CombatTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_CombatBase, CombatTargetBBKey), AActor::StaticClass());
	LastSeenTargetLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_CombatBase, LastSeenTargetLocationBBKey));
}

FBehaviorEvaluator_CombatBase::FBehaviorEvaluator_CombatBase(UBehaviorTreeComponent& OwnerComp,
                                                             const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	CombatConfig = Cast<UBehaviorEvaluatorConfig_CombatBase>(Config);
	NpcImmediateThreatData.Reserve(10);
	EnemyMemoryData.Reserve(10);
}

void FBehaviorEvaluator_CombatBase::PreEvaluate()
{
	Super::PreEvaluate();
	NpcImmediateThreatData.Reset();
	EnemyMemoryData.Reset();
}

float FBehaviorEvaluator_CombatBase::Evaluate()
{
	Super::Evaluate();
	TRACE_CPUPROFILER_EVENT_SCOPE(FBehaviorEvaluator_CombatBase::Evaluate)
	FRelativeOperationContext RelativeOperationData = GetRelativeOperationContext();
	FActorScoresContainer Enemies; 
	const auto& CharactersMemories = PerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& CharacterMemory : CharactersMemories)
	{
		ExecuteEntityOperations(CharacterMemory.Key.Get(), CharacterMemory.Value, RelativeOperationData,
			Enemies, OperationsConfig->EnemiesEvaluationParameters);
		
		const bool bStoreEnemyMemoryData = CharacterMemory.Value.bHostile
			&& GetState() == EBehaviorEvaluatorState::Activated
			&& IsDetectable(CharacterMemory.Value.DetectionSource);
		
		if (bStoreEnemyMemoryData)
		{
			FNpcEnemyCombatMemory EnemyMemoryDataItem;
			EnemyMemoryDataItem.bAlive = CharacterMemory.Value.bAlive;
			EnemyMemoryDataItem.CurrentDetectionSource = CharacterMemory.Value.DetectionSource;
			EnemyMemoryDataItem.LastSeenLocation = CharacterMemory.Key->GetActorLocation();
			EnemyMemoryDataItem.LastUpdateTime = Pawn->GetWorld()->GetTimeSeconds(); 
			EnemyMemoryData.Add(CharacterMemory.Key.Get(), EnemyMemoryDataItem);
		}
	}
	
	float StatePressure = CalculateStatePressure();
	float EnemyPressure = GetEntitiesAggregatedScore(Enemies, OperationsConfig->EnemiesEvaluationParameters);
	
	if (GetState() == EBehaviorEvaluatorState::Activated)
		UpdateAiBehaviorState(Enemies);
	
	return CalculateCombatUtility(EnemyPressure, StatePressure);
}

void FBehaviorEvaluator_CombatBase::OnIndividualScoreCalculated(AActor* Actor, const FCharacterPerceptionData& ActorPerception, float IndividualScore)
{
	if (GetState() != EBehaviorEvaluatorState::Activated)
		return;
	
	if (IsDetectable(ActorPerception.DetectionSource))
		NpcImmediateThreatData.Add(Actor, FNpcImmediateThreatData(IndividualScore, ActorPerception.AttackRange));
}

void FBehaviorEvaluator_CombatBase::UpdateAiBehaviorState(const FActorScoresContainer& PotentialEnemies) const
{
	// already sorted in scores calculation
	// ensure(EnemiesData.bSorted);
	// EnemiesData.PotentialEnemies.Sort();
	CombatLogicComponent->UpdateCombatMemory(EnemyMemoryData);
	CombatLogicComponent->UpdateImmediateThreats(NpcImmediateThreatData);
	
	AActor* BestTarget = nullptr;
	for (const auto& PotentialEnemy : PotentialEnemies)
	{
		// if (PotentialEnemy.bAlive && PotentialEnemy.Score >= 0.f && IsDetectable(PotentialEnemy.DetectionSource))
		// 22 Apr 2026 (aki): removed score >= 0 check. when combat is activated and/or there's no retreat behavior, there can be a following situation:
		// all enemies are too scary for NPC but it is still in combat for whatever valid reasons (i.e. combat started before retreat and there's 10s delay for behavior switch)
		// if there's a score >= 0 check then what ends up happening is NPC ignores actual targets
		if (PotentialEnemy.bAlive && IsDetectable(PotentialEnemy.DetectionSource))
		{
			BestTarget = PotentialEnemy.Actor;
			break;
		}
	}
	
	if (BestTarget != nullptr)
	{
		ensure(BestTarget != Pawn.Get());
		const auto& CurrentPrimaryTargetData = CombatLogicComponent->GetPrimaryTargetData();
		const bool IsUpdateRedundant = CurrentPrimaryTargetData.ActiveTarget == BestTarget 
			&& CurrentPrimaryTargetData.ActiveBehaviorTypeTag == CombatConfig->BehaviorEvaluatorTag;
		if (!IsUpdateRedundant)
		{
	#if WITH_EDITOR
			LogTargetChange(BestTarget, CurrentPrimaryTargetData);
	#endif
			
			CombatLogicComponent->SetCurrentCombatTarget(BestTarget, CombatConfig->BehaviorEvaluatorTag);
			Blackboard->SetValueAsObject(CombatConfig->CombatTargetBBKey.SelectedKeyName, BestTarget);
		}
	}
	else if (CombatLogicComponent->GetPrimaryTargetData().IsValid())
	{
		if (auto LastEnemy = CombatLogicComponent->GetPrimaryTargetActor())
			if (auto LastEnemyMemory = CombatLogicComponent->GetActorCombatMemoryData(LastEnemy))
				if (LastEnemyMemory->bAlive)
					Blackboard->SetValueAsVector(CombatConfig->LastSeenTargetLocationBBKey.SelectedKeyName, LastEnemyMemory->LastSeenLocation);
				
		CombatLogicComponent->ClearCurrentCombatTarget(CombatConfig->BehaviorEvaluatorTag);
		Blackboard->SetValueAsObject(CombatConfig->CombatTargetBBKey.SelectedKeyName, nullptr);
	}
}

float FBehaviorEvaluator_CombatBase::CalculateCombatUtility(float EnemyPressure, float StatePressure)
{
	return EnemyPressure * StatePressure;
}

void FBehaviorEvaluator_CombatBase::OnActivated()
{
	Super::OnActivated();
	CombatLogicComponent->OnCombatBehaviorStarted();
	Evaluate();
}

void FBehaviorEvaluator_CombatBase::Cleanup()
{
	Super::Cleanup();
	if (Blackboard.IsValid() && CombatConfig.IsValid())
	{
		Blackboard->ClearValue(CombatConfig->CombatTargetBBKey.SelectedKeyName);
	}
	
	if (CombatLogicComponent.IsValid())
	{
		CombatLogicComponent->ClearCurrentCombatTarget(CombatConfig->BehaviorEvaluatorTag);
		CombatLogicComponent->ClearEnemiesData();
		CombatLogicComponent->OnCombatBehaviorEnded();
	}
}

bool FBehaviorEvaluator_CombatBase::IsCharacterRelevant(const FCharacterPerceptionData& CharacterPerceptionData,
	const FEntityOperationEvaluationParameters& EvaluationParameters) const
{
	return Super::IsCharacterRelevant(CharacterPerceptionData, EvaluationParameters) 
		&& CharacterPerceptionData.IsHostile();
}

bool IsDetectable(EDetectionSource DetectionSource)
{
	return (DetectionSource & (EDetectionSource::VisualMemory | EDetectionSource::Ally)) != EDetectionSource::None;
}

#if WITH_EDITOR

void FBehaviorEvaluator_CombatBase::LogTargetChange(AActor* BestTarget, const FNpcActiveTargetData& CurrentPrimaryTargetData) const
{
	FGameplayTag NewTargetId;
	if (auto NewTargetAliveInterface = Cast<INpcAliveCreature>(BestTarget))
		NewTargetId = NewTargetAliveInterface->GetTagId_NpcAliveCreature();
			
	UE_VLOG(AIController.Get(), LogARPGAI_BE_Combat, Log, TEXT("Selected new target [%s]"), *NewTargetId.ToString());
	UE_VLOG_LOCATION(AIController.Get(), LogARPGAI_BE_Combat, Log, BestTarget->GetActorLocation(), 25, FColorList::BrightGold,
					 TEXT("New target [%s]"), *NewTargetId.ToString());
			
	if (CurrentPrimaryTargetData.ActiveTarget.IsValid())
	{
		FGameplayTag OldTargetId;
		if (auto OldTargetAliveInterface = Cast<INpcAliveCreature>(CurrentPrimaryTargetData.ActiveTarget))
			OldTargetId = OldTargetAliveInterface->GetTagId_NpcAliveCreature();
				
		UE_VLOG(AIController.Get(), LogARPGAI_BE_Combat, Log, TEXT("Old target [%s]"), *OldTargetId.ToString());
		UE_VLOG_LOCATION(AIController.Get(), LogARPGAI_BE_Combat, Log, CurrentPrimaryTargetData.ActiveTarget->GetActorLocation(), 25, FColorList::OldGold,
						 TEXT("Old target [%s]"), *OldTargetId.ToString());
	}
	else
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE_Combat, Log, TEXT("No target before"));
	}
}

#endif
