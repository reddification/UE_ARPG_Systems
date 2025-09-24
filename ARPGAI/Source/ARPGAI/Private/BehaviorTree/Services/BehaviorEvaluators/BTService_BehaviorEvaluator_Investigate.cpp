#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_Investigate.h"

#include "AIController.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"

UBTService_BehaviorEvaluator_Investigate::UBTService_BehaviorEvaluator_Investigate()
{
	NodeName = "Behavior evaluator: investigate";
	InvestigateLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_Investigate, InvestigateLocationBBKey));
}

void UBTService_BehaviorEvaluator_Investigate::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(NodeMemory);
	float UtilityAccumulation = UpdatePerception(OwnerComp, BTMemory);
	ChangeUtility(UtilityAccumulation, OwnerComp.GetBlackboardComponent(), DeltaSeconds, BTMemory);
}

void UBTService_BehaviorEvaluator_Investigate::InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	Super::InitiateBehaviorState(BTComponent);
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	UpdatePerception(*BTComponent, BTMemory);
}

float UBTService_BehaviorEvaluator_Investigate::UpdatePerception(UBehaviorTreeComponent& OwnerComp, const FBTMemory_BehaviorEvaluator_Base* BTMemory) const
{
#if WITH_EDITOR
	if (!BTMemory->PerceptionComponent.IsValid())
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Error, TEXT("Shit's fucked up no BTMemory in investigation perception update"));
		return 0.f;
	}
#endif
	
	const auto& HeardSounds = BTMemory->PerceptionComponent->GetHeardSounds();

	auto Blackboard = OwnerComp.GetBlackboardComponent();
	float InvestigationDesire = BTMemory->GetUtilityRegressionOffset();
	TArray<FInvestigationCandidate> InvestigationCandidates;
	auto DistanceDependency = AttractingSoundToDistanceDependencyCurve.GetRichCurveConst();
	for (const auto& HeardSound : HeardSounds)
	{
		if (!AttractingSounds.HasTag(HeardSound.Value.SoundTag))
			continue;

		float LocalUtilityAccumulation = DistanceDependency->Eval(HeardSound.Value.Distance);
		if (HeardSound.Value.bByAlly)
			LocalUtilityAccumulation *= ByAllyModifier;

		InvestigationCandidates.Add({ HeardSound.Value.Location, LocalUtilityAccumulation });

		InvestigationDesire += LocalUtilityAccumulation;
	}

	if (BTMemory->bActive && !InvestigationCandidates.IsEmpty())
	{
		if (InvestigationCandidates.Num() > 1)
			InvestigationCandidates.Sort();
		
		Blackboard->SetValueAsVector(InvestigateLocationBBKey.SelectedKeyName, InvestigationCandidates[0].Location);
	}
	
	return InvestigationDesire;
}

FString UBTService_BehaviorEvaluator_Investigate::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}
