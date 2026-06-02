#include "BehaviorTree/Services/BTService_CountActiveCombatants.h"
#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Subsystems/NpcSquadSubsystem.h"

UBTService_CountActiveCombatants::UBTService_CountActiveCombatants()
{
	NodeName = "Count active combatants";
	bNotifyCeaseRelevant = true;
	OutAliveAlliesCountBBKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CountActiveCombatants, OutAliveAlliesCountBBKey));
	OutDeadAlliesCountBBKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CountActiveCombatants, OutDeadAlliesCountBBKey));
	OutAliveEnemiesCountBBKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CountActiveCombatants, OutAliveEnemiesCountBBKey));
	OutDeadEnemiesCountBBKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CountActiveCombatants, OutDeadEnemiesCountBBKey));
}

void UBTService_CountActiveCombatants::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_CountActiveCombatants)
	
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	
	FCountAliveCharactersResult Result = bUsePerception 
		? CountCharacters_Perception(OwnerComp, Blackboard)
		: CountCharacters_Knowledge(OwnerComp, Blackboard);
	
	Blackboard->SetValueAsInt(OutAliveAlliesCountBBKey.SelectedKeyName, Result.AliveAlliesCount);
	Blackboard->SetValueAsInt(OutDeadAlliesCountBBKey.SelectedKeyName, Result.DeadAlliesCount);
	Blackboard->SetValueAsInt(OutAliveEnemiesCountBBKey.SelectedKeyName, Result.AliveEnemiesCount);
	Blackboard->SetValueAsInt(OutDeadEnemiesCountBBKey.SelectedKeyName, Result.DeadEnemiesCount);
}

void UBTService_CountActiveCombatants::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->ClearValue(OutAliveAlliesCountBBKey.SelectedKeyName);
		Blackboard->ClearValue(OutDeadAlliesCountBBKey.SelectedKeyName);
		Blackboard->ClearValue(OutAliveEnemiesCountBBKey.SelectedKeyName);
		Blackboard->ClearValue(OutDeadEnemiesCountBBKey.SelectedKeyName);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

UBTService_CountActiveCombatants::FCountAliveCharactersResult UBTService_CountActiveCombatants::CountCharacters_Perception(
	const UBehaviorTreeComponent& OwnerComp, const UBlackboardComponent* Blackboard)
{
	FCountAliveCharactersResult Result;
	const float AliveAllyDistanceThreshold = AliveAllyRelevantDistance.GetValue(Blackboard);
	const float DeadAllyDistanceThreshold = DeadAllyRelevantDistance.GetValue(Blackboard);
	const float AliveEnemyDistanceThreshold = AliveEnemyRelevantDistance.GetValue(Blackboard);
	const float DeadEnemyDistanceThreshold = DeadEnemyRelevantDistance.GetValue(Blackboard);

	auto PerceptionComponent = Cast<UNpcPerceptionComponent>(OwnerComp.GetAIOwner()->GetAIPerceptionComponent());
	const auto& Characters = PerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& Character : Characters)
	{
		if (Character.Value.bAlly)
		{
			if (Character.Value.bAlive)
			{
				if (Character.Value.Distance <= AliveAllyDistanceThreshold)
					Result.AliveAlliesCount++;
			}
			else if (Character.Value.Distance <= DeadAllyDistanceThreshold)
				Result.DeadAlliesCount++;			
		}
		else if (Character.Value.bHostile)
		{
			if (Character.Value.bAlive)
			{
				if (Character.Value.Distance <= AliveEnemyDistanceThreshold)
					Result.AliveEnemiesCount++;
			}
			else if (Character.Value.Distance <= DeadEnemyDistanceThreshold)
				Result.DeadEnemiesCount++;
		}
	}
	
	return Result;
}

UBTService_CountActiveCombatants::FCountAliveCharactersResult UBTService_CountActiveCombatants::CountCharacters_Knowledge(
	const UBehaviorTreeComponent& OwnerComp, const UBlackboardComponent* Blackboard)
{
	FCountAliveCharactersResult Result;
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto SquadSubsystem = UNpcSquadSubsystem::Get(Pawn);
	auto Allies = SquadSubsystem->GetAllies(Pawn.Get(), false);
	FVector NpcLocation = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
	
	if (!Allies.IsEmpty())
	{
		const float AliveAllyDistanceThreshold = FMath::Square(AliveAllyRelevantDistance.GetValue(Blackboard));
		const float DeadAllyDistanceThreshold = FMath::Square(DeadAllyRelevantDistance.GetValue(Blackboard));

		for (const auto* Ally : Allies)
		{
			bool bAlive = true;
			if (auto AliveInterface = Cast<INpcAliveCreature>(Ally))
				bAlive = AliveInterface->IsAlive_NpcAliveCreature();

			const float DistSq = (NpcLocation - Ally->GetActorLocation()).SizeSquared();
			if (bAlive)
			{
				if (DistSq <= AliveAllyDistanceThreshold)
					Result.AliveAlliesCount++;
			}
			else if (DistSq <= DeadAllyDistanceThreshold)
				Result.DeadAlliesCount++;
		}
	}
	
	auto NpcCombatLogicComponent = GetNpcCombatLogicComponent(OwnerComp);
	const auto& CombatEnemiesMemory = NpcCombatLogicComponent->GetCombatEnemiesMemory();
	if (!CombatEnemiesMemory.IsEmpty())
	{
		const float AliveEnemyDistanceThreshold = FMath::Square(AliveEnemyRelevantDistance.GetValue(Blackboard));
		const float DeadEnemyDistanceThreshold = FMath::Square(DeadEnemyRelevantDistance.GetValue(Blackboard));
		
		for (const auto& EnemyMemory : CombatEnemiesMemory)
		{
			const float DistSq = (EnemyMemory.Value.LastSeenLocation - NpcLocation).SizeSquared();
			if (EnemyMemory.Value.bAlive)
			{
				if (DistSq <= AliveEnemyDistanceThreshold)
					Result.AliveEnemiesCount++;
			}
			else if (DistSq <= DeadEnemyDistanceThreshold)
				Result.DeadEnemiesCount++;
		}
	}
	
	return Result;
}

FString UBTService_CountActiveCombatants::GetStaticDescription() const
{
	return FString::Printf(TEXT("Using %s, Count\nAlive allies in range %s to %s\nDead allies in range %s to %s\nAlive enemies in range %s to %s\nDead enemies in range %s to %s\n%s"),
		bUsePerception ? TEXT("Perception") : TEXT("Memory"),
		*AliveAllyRelevantDistance.ToString(), *OutAliveAlliesCountBBKey.SelectedKeyName.ToString(),
		*DeadAllyRelevantDistance.ToString(), *OutDeadAlliesCountBBKey.SelectedKeyName.ToString(),
		*AliveEnemyRelevantDistance.ToString(), *OutAliveEnemiesCountBBKey.SelectedKeyName.ToString(),
		*DeadEnemyRelevantDistance.ToString(), *OutDeadEnemiesCountBBKey.SelectedKeyName.ToString(),
		*Super::GetStaticDescription());
}
