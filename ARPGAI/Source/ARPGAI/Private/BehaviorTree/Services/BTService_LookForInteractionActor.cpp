// 


#include "BehaviorTree/Services/BTService_LookForInteractionActor.h"

#include "AIController.h"
#include "GameplayTagAssetInterface.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcPerceptionComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTService_LookForInteractionActor::UBTService_LookForInteractionActor()
{
	NodeName = "Look for interaction actor";
	OutInteractionActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_LookForInteractionActor, OutInteractionActorBBKey), AActor::StaticClass());
}

void UBTService_LookForInteractionActor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_LookForInteractionActor::TickNode)
	
	auto PerceptionComponent = Cast<UNpcPerceptionComponent>(OwnerComp.GetAIOwner()->GetAIPerceptionComponent());
	TArray<AActor*> CurrentlyPerceivedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), CurrentlyPerceivedActors);
	AActor* FoundTarget = nullptr;
	for (auto* PerceivedActor : CurrentlyPerceivedActors)
	{
		auto GameplayTagInterface = Cast<IGameplayTagAssetInterface>(PerceivedActor);
		if (!GameplayTagInterface)
			continue;

		if (PerceptionComponent->GetAccumulatedTimeSeen(PerceivedActor) < MinRequiredTimeSeen)
			continue;
		
		FGameplayTagContainer ActorTags;
		GameplayTagInterface->GetOwnedGameplayTags(ActorTags);

		if (bUseInteractionActorId && InteractionActorId.IsValid())
		{
			if (ActorTags.HasTagExact(InteractionActorId))
			{
				FoundTarget = PerceivedActor;
				break;
			}
		}
		else if (!InteractionActorQuery.IsEmpty() && InteractionActorQuery.Matches(ActorTags))
		{
			FoundTarget = PerceivedActor;
			break;
		}
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsObject(OutInteractionActorBBKey.SelectedKeyName, FoundTarget);
}

FString UBTService_LookForInteractionActor::GetStaticDescription() const
{
	FString ActorDescription = bUseInteractionActorId
			? FString::Printf(TEXT("Find actor with id %s"), *InteractionActorId.ToString())
			: FString::Printf(TEXT("Find target that matches query: %s"), *InteractionActorQuery.GetDescription());
	
	return FString::Printf(TEXT("[out] Target BB: %s\nMin time seen = %.2f\n%s\n%s"),
		*OutInteractionActorBBKey.SelectedKeyName.ToString(), MinRequiredTimeSeen, *ActorDescription, *Super::GetStaticDescription());
}
