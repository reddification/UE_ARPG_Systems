// 


#include "BehaviorEvaluators/BehaviorEvaluator_UnexpectedDamage.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"

void UBehaviorEvaluator_UnexpectedDamage::Activate(AAIController* InAIController)
{
	Super::Activate(InAIController);
	auto PerceptionComponent = AIController->GetAIPerceptionComponent();
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &UBehaviorEvaluator_UnexpectedDamage::OnPerceptionUpdated);
	ensure(!ReceivedHitFromLocationBBKey.SelectedKeyName.IsNone());
}

void UBehaviorEvaluator_UnexpectedDamage::Deactivate()
{
	if (AIController.IsValid())
	{
		auto PerceptionComponent = AIController->GetAIPerceptionComponent();
		PerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
	}
	
	Super::Deactivate();
}

void UBehaviorEvaluator_UnexpectedDamage::OnPerceptionUpdated(AActor* TriggerActor, FAIStimulus Stimulus)
{
	if (!ensure(TriggerActor != nullptr)) 
		return;
	
	if (!IsHostile(TriggerActor) || Stimulus.IsExpired() || !Stimulus.IsActive())
		return;
	
	const FAISenseID DamageSenseID = UAISense::GetSenseID(UAISense_Damage::StaticClass());
	if (Stimulus.Type == DamageSenseID)
	{
		if (!ReceivedHitFromLocationBBKey.SelectedKeyName.IsNone())
			BlackboardComponent->SetValueAsVector(ReceivedHitFromLocationBBKey.SelectedKeyName, TriggerActor->GetActorLocation());
	}

	ChangeUtility(UtilityFromPerceivingDamage.GetValue(BlackboardComponent.Get()));
}
