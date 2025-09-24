// 
#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_ProximityAwareness.h"

#include "AIController.h"
#include "Components/NpcProximityAwarenessComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Perception/AIPerceptionComponent.h"

UBTService_BehaviorEvaluator_ProximityAwareness::UBTService_BehaviorEvaluator_ProximityAwareness()
{
	NodeName = "Behavior evaluator: proximity awareness";
	AwarenessLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_ProximityAwareness, AwarenessLocationBBKey));
}

void UBTService_BehaviorEvaluator_ProximityAwareness::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                   float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	auto AIController = OwnerComp.GetAIOwner();
	auto PerceptionComponent = Cast<UNpcPerceptionComponent>(AIController->GetAIPerceptionComponent());
	auto BTMemory = reinterpret_cast<FBTMemory_BE_WakeUp*>(NodeMemory);

#if WITH_EDITOR
	if (!BTMemory->PerceptionComponent.IsValid())
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Error, TEXT("Shit's fucked up no BTMemory in proximity awareness perception update"));
		return;
	}
#endif
	
	auto Pawn = AIController->GetPawn();
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	float WakeUpDesire = BTMemory->GetUtilityRegressionOffset();
	float IncrementValue = BTMemory->GetUtilityAccumulationRate();
	if (auto NpcProximityComponent = Pawn->FindComponentByClass<UNpcProximityAwarenessComponent>())
	{
		auto ActorsInProximity = NpcProximityComponent->GetDetectedActorsInProximity();
		if (ActorsInProximity.Num() > 0)
		{
			BTMemory->AwarenessLocation = ActorsInProximity[0]->GetActorLocation();
			WakeUpDesire += IncrementValue;
		}
	}

	auto HeardSounds = PerceptionComponent->GetHeardSounds();
	for (const auto& HeardSound : HeardSounds)
	{
		if (WakeUpToNoises.HasTag(HeardSound.Value.SoundTag) && !HeardSound.Value.bByAlly && FMath::RandRange(0.f, 1.f) < WakeUpToNoiseChance)
		{
			BTMemory->AwarenessLocation = HeardSound.Value.Location;
			WakeUpDesire += IncrementValue;
		}
	}
	
	ChangeUtility(WakeUpDesire, OwnerComp.GetBlackboardComponent(), DeltaSeconds, BTMemory);
}

void UBTService_BehaviorEvaluator_ProximityAwareness::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto ProximityAwarenessComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcProximityAwarenessComponent>();
	if (ProximityAwarenessComponent)
		ProximityAwarenessComponent->Activate();
}

void UBTService_BehaviorEvaluator_ProximityAwareness::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Pawn = AIController->GetPawn())
			if (auto ProximityAwarenessComponent = Pawn->FindComponentByClass<UNpcProximityAwarenessComponent>())
				ProximityAwarenessComponent->Deactivate();
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_BehaviorEvaluator_ProximityAwareness::InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::InitiateBehaviorState(BTComponent);
	auto BTMemory = reinterpret_cast<FBTMemory_BE_WakeUp*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	BTComponent->GetBlackboardComponent()->SetValueAsVector(AwarenessLocationBBKey.SelectedKeyName, BTMemory->AwarenessLocation);
}

void UBTService_BehaviorEvaluator_ProximityAwareness::FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	auto BTMemory = reinterpret_cast<FBTMemory_BE_WakeUp*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	BTComponent->GetBlackboardComponent()->ClearValue(AwarenessLocationBBKey.SelectedKeyName);
	BTMemory->AwarenessLocation = FAISystem::InvalidLocation;
	Super::FinalizeBehaviorState(BTComponent);
}

FString UBTService_BehaviorEvaluator_ProximityAwareness::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out]Awareness location BB: %s\nChance to hear step: %.2f\n%s"), *AwarenessLocationBBKey.SelectedKeyName.ToString(),
		WakeUpToNoiseChance, *Super::GetStaticDescription());
}
