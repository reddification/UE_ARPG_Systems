#include "BehaviorEvaluators/BehaviorEvaluator_SuddenThreat.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcThreat.h"
#include "Perception/AISense_Damage.h"

UBehaviorEvaluatorConfig_SuddenThreatReact::UBehaviorEvaluatorConfig_SuddenThreatReact()
{
	ReceivedHitFromLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_SuddenThreatReact, ReceivedHitFromLocationBBKey));
	AttackerActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_SuddenThreatReact, AttackerActorBBKey), AActor::StaticClass());
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_SuddenThreatReact::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_SuddenThreat>(*BTComponent, this);
}

FBehaviorEvaluator_SuddenThreat::FBehaviorEvaluator_SuddenThreat(UBehaviorTreeComponent& OwnerComp,
	const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	SuddenThreatConfig = Cast<UBehaviorEvaluatorConfig_SuddenThreatReact>(Config);
}

FBehaviorEvaluator_SuddenThreat::~FBehaviorEvaluator_SuddenThreat()
{
	UnsubscribeFromPerceptionChanges();
}

void FBehaviorEvaluator_SuddenThreat::SubscribeToPerceptionChanges()
{
	if (ensure(!PerceptionUpdateDelegateHandle.IsValid()))
	{
		PerceptionUpdateDelegateHandle = PerceptionComponent->TargetPerceptionUpdatedNativeEvent.AddRaw(this, 
			&FBehaviorEvaluator_SuddenThreat::OnPerceptionUpdated);
	}
}

void FBehaviorEvaluator_SuddenThreat::UnsubscribeFromPerceptionChanges()
{
	if (PerceptionComponent.IsValid() && PerceptionUpdateDelegateHandle.IsValid())
	{
		PerceptionComponent->TargetPerceptionUpdatedNativeEvent.Remove(PerceptionUpdateDelegateHandle);
		PerceptionUpdateDelegateHandle.Reset();
	}
}

void FBehaviorEvaluator_SuddenThreat::SetState(EBehaviorEvaluatorState NewState)
{
	auto OldState = GetState();
	Super::SetState(NewState);
	if (OldState == NewState)
		return;
	
	if (NewState == EBehaviorEvaluatorState::Relevant)
		SubscribeToPerceptionChanges();
	
	if (OldState == EBehaviorEvaluatorState::Relevant)
		UnsubscribeFromPerceptionChanges();
}

void FBehaviorEvaluator_SuddenThreat::OnPerceptionUpdated(AActor* TriggerActor, const FAIStimulus& Stimulus)
{
	if (GetState() != EBehaviorEvaluatorState::Relevant)
		return;
	
	if (Stimulus.IsExpired() || !Stimulus.IsActive())
		return;

	if (!Pawn.IsValid() || !SuddenThreatConfig.IsValid())
		return;
	
	const FAISenseID DamageSenseID = UAISense::GetSenseID(UAISense_Damage::StaticClass());
	if (Stimulus.Type == DamageSenseID)
	{
		if (SuddenThreatConfig->bIgnoreVisuallyPerceivedCauses)
			if (const auto* KnownCharacterData = PerceptionComponent->GetShortTermCharactersMemory(TriggerActor))
				if (KnownCharacterData->HasImmediateVisualDetection(0.5f))
					return;
		
		auto NpcAttitudesComponent = GetNpcAttitudesComponent(Pawn.Get());
		AttackedFromLocation = TriggerActor->GetActorLocation();
		if (NpcAttitudesComponent->IsHostile(TriggerActor))
			if (CanObserve(Pawn.Get(), TriggerActor, *SuddenThreatConfig.Get()))
				Attacker = TriggerActor;

		SetMaxUtility();
	}
}

void FBehaviorEvaluator_SuddenThreat::OnActivated()
{
	Super::OnActivated();
	if (ensure(Blackboard.IsValid() && SuddenThreatConfig.IsValid()))
	{
		Blackboard->SetValueAsVector(SuddenThreatConfig->ReceivedHitFromLocationBBKey.SelectedKeyName, AttackedFromLocation);
		Blackboard->SetValueAsObject(SuddenThreatConfig->AttackerActorBBKey.SelectedKeyName, Attacker.Get());
		if (AttackedFromLocation != FVector::ZeroVector && AttackedFromLocation != FAISystem::InvalidLocation)
		{
			UE_VLOG_LOCATION(AIController.Get(), LogARPGAI_BE, Log, AttackedFromLocation, 25, FColorList::DarkOliveGreen, TEXT("Sudden thread: received hit from location"));
		}
	}
	
	if (Attacker.IsValid())
	{
		FNpcCurrentCombatThreatsContainer ActiveThreatsContainer;
		float AttackRange = 100.f;
		if (auto AttackerThreatInterface = Cast<INpcThreat>(Attacker.Get()))
			AttackRange = AttackerThreatInterface->GetAttackRange_NpcThreat();
		
		ActiveThreatsContainer.Add(Attacker, FNpcImmediateThreatData(1.f, AttackRange));
		CombatLogicComponent->UpdateImmediateThreats(ActiveThreatsContainer);
	}
}

void FBehaviorEvaluator_SuddenThreat::Cleanup()
{
	Super::Cleanup();
	if (Blackboard.IsValid() && SuddenThreatConfig.IsValid())
	{
		Blackboard->ClearValue(SuddenThreatConfig->ReceivedHitFromLocationBBKey.SelectedKeyName);
		Blackboard->ClearValue(SuddenThreatConfig->AttackerActorBBKey.SelectedKeyName);
	}
		
	AttackedFromLocation = FAISystem::InvalidLocation;
	Attacker.Reset();
	
	if (CombatLogicComponent.IsValid())
		CombatLogicComponent->UpdateImmediateThreats({});
}

bool FBehaviorEvaluator_SuddenThreat::CanObserve(const APawn* NpcPawn, const AActor* AttackerActor, const UBehaviorEvaluatorConfig_SuddenThreatReact& Config) const
{
	const float DistanceSq = (NpcPawn->GetActorLocation() - AttackerActor->GetActorLocation()).SizeSquared();
	return DistanceSq < Config.PerceiveDamageCauserDistanceThreshold * Config.PerceiveDamageCauserDistanceThreshold;
	// I assume in future you could also check for stealth features, like invisibility, surrounding lighting, smoke, etc
}
