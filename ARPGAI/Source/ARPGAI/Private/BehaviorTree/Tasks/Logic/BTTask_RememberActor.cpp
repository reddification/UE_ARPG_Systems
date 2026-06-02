#include "BehaviorTree/Tasks/Logic/BTTask_RememberActor.h"

#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcMemoryComponent.h"

UBTTask_RememberActor::UBTTask_RememberActor()
{
	NodeName = "Remember actor";
	ActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RememberActor, ActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_RememberActor::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto AttitudesComponent = GetNpcLongTermMemoryComponent(OwnerComp);
	if (AttitudesComponent == nullptr)
		return EBTNodeResult::Failed;
	
	auto Actor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ActorKey.SelectedKeyName));
	if (Actor == nullptr)
		return EBTNodeResult::Failed;
	
	if (bRememberForever)
		AttitudesComponent->RememberActorTraits(Actor, RememberTraits);
	else 
		AttitudesComponent->RememberActorTraits(Actor, RememberTraits, RememberForDurationGTH);
		
	return EBTNodeResult::Succeeded;
}

FString UBTTask_RememberActor::GetStaticDescription() const
{
	if (RememberTraits.IsEmpty())
		return TEXT("Traits not set");
	
	FString TraitsList;
	for (const auto& Trait : RememberTraits)
		TraitsList += FString::Printf(TEXT("\n%s"), *Trait.ToString());
	
	FString DurationString = bRememberForever ? TEXT("Forever") : FString::Printf(TEXT("For %.2f game time hours"), RememberForDurationGTH);
	
	return FString::Printf(TEXT("Remember traits for %s:%s\n%s"), 
		*ActorKey.SelectedKeyName.ToString(), *TraitsList, *DurationString);
}
