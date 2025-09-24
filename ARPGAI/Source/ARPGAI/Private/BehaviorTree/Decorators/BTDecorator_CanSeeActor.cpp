// 


#include "BehaviorTree/Decorators/BTDecorator_CanSeeActor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTDecorator_CanSeeActor::UBTDecorator_CanSeeActor()
{
	NodeName = "Can see actor";
	ActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CanSeeActor, ActorBBKey), AActor::StaticClass());
}

bool UBTDecorator_CanSeeActor::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	auto Actor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ActorBBKey.SelectedKeyName));
	if (!Actor)
		return false;
	
	auto AIPerception = OwnerComp.GetAIOwner()->GetAIPerceptionComponent();
	FActorPerceptionBlueprintInfo ActorPerceptionInfo;
	bool bHasPerception = AIPerception->GetActorsPerception(Actor, ActorPerceptionInfo);
	if (!bHasPerception)
		return false;

	auto SightSenseId = UAISense::GetSenseID(UAISense_Sight::StaticClass());
	for (const auto& Stimuli : ActorPerceptionInfo.LastSensedStimuli)
	{
		if (Stimuli.Type == SightSenseId)
		{
			if (Stimuli.IsActive())
			{
				ensure(!Stimuli.IsExpired());
			}
			
			return Stimuli.IsActive();// && !Stimuli.IsExpired(); // IsExpired check might be redundant
		}
	}

	return false;
}

FString UBTDecorator_CanSeeActor::GetStaticDescription() const
{
	return FString::Printf(TEXT("Actor: %s\n%s"), *ActorBBKey.SelectedKeyName.ToString(), *Super::GetStaticDescription()) ;
}
