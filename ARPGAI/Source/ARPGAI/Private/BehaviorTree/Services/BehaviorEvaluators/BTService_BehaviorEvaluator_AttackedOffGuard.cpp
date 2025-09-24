// 


#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_AttackedOffGuard.h"

#include "AIController.h"
#include "Activities/ActivityInstancesHelper.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Subsystems/NpcSquadSubsystem.h"

UBTService_BehaviorEvaluator_AttackedOffGuard::UBTService_BehaviorEvaluator_AttackedOffGuard()
{
	NodeName = "Behavior evaluator: attacked off guard";
	ReceivedHitFromLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_AttackedOffGuard, ReceivedHitFromLocationBBKey));
	AttackerActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_AttackedOffGuard, AttackerActorBBKey), AActor::StaticClass());
}

void UBTService_BehaviorEvaluator_AttackedOffGuard::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp,
                                                                     uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto AIController = OwnerComp.GetAIOwner();
	auto PerceptionComponent = Cast<UNpcPerceptionComponent>(AIController->GetAIPerceptionComponent());
	PerceptionComponent->TargetPerceptionUpdatedNativeEvent.AddUObject(this, &UBTService_BehaviorEvaluator_AttackedOffGuard::OnPerceptionUpdated, &OwnerComp);
}

void UBTService_BehaviorEvaluator_AttackedOffGuard::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (auto AIController = OwnerComp.GetAIOwner())
	{
		auto PerceptionComponent = Cast<UNpcPerceptionComponent>(AIController->GetAIPerceptionComponent());
		PerceptionComponent->TargetPerceptionUpdatedNativeEvent.RemoveAll(this);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_BehaviorEvaluator_AttackedOffGuard::InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::InitiateBehaviorState(BTComponent);
	auto NodeMemoryRaw = BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this));
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_AttackedOffGuard*>(NodeMemoryRaw);
	auto Blackboard = BTComponent->GetBlackboardComponent();
	Blackboard->SetValueAsVector(ReceivedHitFromLocationBBKey.SelectedKeyName, BTMemory->AttackedFromLocation);
	Blackboard->SetValueAsObject(AttackerActorBBKey.SelectedKeyName, BTMemory->Attacker.Get());
}

void UBTService_BehaviorEvaluator_AttackedOffGuard::FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::FinalizeBehaviorState(BTComponent);
	BTComponent->GetBlackboardComponent()->ClearValue(ReceivedHitFromLocationBBKey.SelectedKeyName);
}

void UBTService_BehaviorEvaluator_AttackedOffGuard::OnPerceptionUpdated(AActor* TriggerActor, const FAIStimulus& Stimulus,
                                                                        UBehaviorTreeComponent* BehaviorTreeComponent)
{
	auto NodeMemoryRaw = BehaviorTreeComponent->GetNodeMemory(this, BehaviorTreeComponent->FindInstanceContainingNode(this));
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_AttackedOffGuard*>(NodeMemoryRaw);
	if (Stimulus.IsExpired() || !Stimulus.IsActive())
		return;
	
	const FAISenseID DamageSenseID = UAISense::GetSenseID(UAISense_Damage::StaticClass());
	if (Stimulus.Type == DamageSenseID)
	{
		BTMemory->AttackedFromLocation = TriggerActor->GetActorLocation();
		if (CanObserve(BehaviorTreeComponent->GetAIOwner()->GetPawn(), TriggerActor))
		{
			auto NpcAttitudesComponent = GetNpcAttitudesComponent(*BehaviorTreeComponent);
			NpcAttitudesComponent->SetHostile(TriggerActor, true, true);
			BTMemory->Attacker = TriggerActor;
		}

		auto BlackboardComponent = BehaviorTreeComponent->GetBlackboardComponent();
		ChangeUtility(MaxUtility.GetValue(BlackboardComponent), BlackboardComponent, 1.f, BTMemory);
	}
}

bool UBTService_BehaviorEvaluator_AttackedOffGuard::CanObserve(const APawn* NpcPawn, const AActor* AttackerActor) const
{
	const float DistanceSq = (NpcPawn->GetActorLocation() - AttackerActor->GetActorLocation()).SizeSquared();
	return DistanceSq < PerceiveDamageCauserDistanceThreshold * PerceiveDamageCauserDistanceThreshold;
	// I assume in future you could also check for stealth features, like invisibility, surrounding lighting, smoke, etc
}

void UBTService_BehaviorEvaluator_AttackedOffGuard::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	// BehaviorEvaluatorTag
}

FString UBTService_BehaviorEvaluator_AttackedOffGuard::GetStaticDescription() const
{
	return FString::Printf(TEXT("Attacked from direction BB: %s\n%s"), *ReceivedHitFromLocationBBKey.SelectedKeyName.ToString(),
		*Super::GetStaticDescription());
}
