#include "ReactionEvaluators/NpcReactionEvaluator_ToCharacter.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "GameplayTagAssetInterface.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcPerceptionReactionComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

float UNpcReactionEvaluator_ToCharacter::ProcessPerceptionInternal(AAIController* NpcController,
                                                                   UObject* ReactionEvaluatorMemory,
                                                                   float DeltaTime) const
{
	Super::ProcessPerceptionInternal(NpcController, ReactionEvaluatorMemory, DeltaTime);

	auto PerceptionComponent = NpcController->GetAIPerceptionComponent();

	auto NpcComponent = NpcController->GetPawn()->FindComponentByClass<UNpcAttitudesComponent>();
	TArray<FReactionCauserData> ReactionCausers;

	const FVector& NpcPawnLocation = NpcController->GetPawn()->GetActorLocation();
	const float ReactionRelevancyDistanceLimitSq = ReactionRelevancyDistanceLimit * ReactionRelevancyDistanceLimit;
	TArray<AActor*> SeenActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);
	for (auto TargetActor : SeenActors)
	{
		if (auto AliveCreature = Cast<INpcAliveCreature>(TargetActor))
			if (!AliveCreature->IsNpcActorAlive())
				continue;
		
		const float DistSq = (NpcPawnLocation - TargetActor->GetActorLocation()).SizeSquared();
		if (DistSq > ReactionRelevancyDistanceLimitSq)
			continue;

		if (!OnlyReactToCharactersWithAttitudes.IsEmpty())
		{
			FGameplayTag AttitudeToCharacter = NpcComponent->GetAttitude(TargetActor);
			if (!OnlyReactToCharactersWithAttitudes.HasTag(AttitudeToCharacter))
				continue;
		}
		
		if (auto TagsActor = Cast<IGameplayTagAssetInterface>(TargetActor))
		{
			FGameplayTagContainer PerceivedActorTags;
			TagsActor->GetOwnedGameplayTags(PerceivedActorTags);
			if (CharacterTagsFilter.Matches(PerceivedActorTags))
				ReactionCausers.Add({ TargetActor, DistSq });
		}
	}

	if (ReactionCausers.Num() > 1)
	{
		ReactionCausers.Sort([](const FReactionCauserData& A, const FReactionCauserData& B){ return A.DistSq < B.DistSq; });
		ensure(ReactionCausers[0].DistSq < ReactionCausers[1].DistSq); // i don't remember if it predicate should have less than or greater than for sorting so just ensuring
	}

	auto ActorReactionMemory = Cast<UNpcReactionEvaluatorMemory_Actor>(ReactionEvaluatorMemory);
	if (!ReactionCausers.IsEmpty())
		ActorReactionMemory->Actor = ReactionCausers[0].Actor;

	const float DeltaUtility = ReactionCausers.Num() > 0 ? UtilityAccumulationRate * DeltaTime : -UtilityDecayRate * DeltaTime;
	if (DeltaUtility > 0.f && ChanceToIncreaseUtility < 1.f && FMath::RandRange(0.f, 1.f) > ChanceToIncreaseUtility)
		return 0.f;
	
	return DeltaUtility;
}

bool UNpcReactionEvaluator_ToCharacter::LoadReactionContext(const UNpcBlackboardDataAsset* BlackboardKeys, UBlackboardComponent* BlackboardComponent,
	UObject* ReactionEvaluatorMemory) const
{
	bool bLoadedBaseContext = Super::LoadReactionContext(BlackboardKeys, BlackboardComponent, ReactionEvaluatorMemory);
	if (!bLoadedBaseContext)
		return false;

	auto Memory = Cast<UNpcReactionEvaluatorMemory_Actor>(ReactionEvaluatorMemory);
	
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->ReactionSpeechBBKey.SelectedKeyName, NpcReactionSpeechTag.GetSingleTagContainer());
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->ReactionGestureBBKey.SelectedKeyName, GestureTag.GetSingleTagContainer());
	BlackboardComponent->SetValueAsBool(BlackboardKeys->ReactionIsIndefiniteBBKey.SelectedKeyName, bReactionIndefinite);
	BlackboardComponent->SetValueAsFloat(BlackboardKeys->ReactionTimeLimitBBKey.SelectedKeyName, ReactionMaxTime);
	
	if (!ReactionEQS.IsNull())
		BlackboardComponent->SetValueAsObject(BlackboardKeys->ReactionEQSBBKey.SelectedKeyName, ReactionEQS.LoadSynchronous());
	else
		BlackboardComponent->ClearValue(BlackboardKeys->ReactionEQSBBKey.SelectedKeyName);

	if (Memory->Actor.IsValid())
		BlackboardComponent->SetValueAsObject(BlackboardKeys->ReactionCauserActorBBKey.SelectedKeyName, const_cast<AActor*>(Memory->Actor.Get()));
	else
		BlackboardComponent->ClearValue(BlackboardKeys->ReactionCauserActorBBKey.SelectedKeyName);

	return true;
}

UObject* UNpcReactionEvaluator_ToCharacter::CreateMemory(UObject* OuterObject)
{
	return NewObject<UNpcReactionEvaluatorMemory_Actor>(OuterObject);
}

void UNpcReactionEvaluator_ToCharacter::CompleteReaction(UNpcPerceptionReactionComponent* NpcPerceptionReactionComponent, UBlackboardComponent* Blackboard, UObject* ReactionEvaluatorMemory, const
                                                         FGameplayTag& ReactionBehaviorExecutionResult) const
{
	// 30.11.2024 @AK: Ideally, NpcReactionEvaluator shouldn't know anything about NpcComponent
	// But ATM IDK how to program it in a polymorphic manner, because this evaluator needs an option to update an attitude to a character after completion,
	// but some other reaction evaluators - don't
	Super::CompleteReaction(NpcPerceptionReactionComponent, Blackboard, ReactionEvaluatorMemory, ReactionBehaviorExecutionResult);
	auto Memory = Cast<UNpcReactionEvaluatorMemory_Actor>(ReactionEvaluatorMemory);
	if (ReactionBehaviorExecutionResult == AIGameplayTags::AI_ReactionEvaluator_ExecutionResult_Success)
	{
		if (NewAttitudeAfterReactionComplete.IsValid())
		{
			auto NpcComponent = NpcPerceptionReactionComponent->GetOwner()->FindComponentByClass<UNpcAttitudesComponent>();
			NpcComponent->AddTemporaryCharacterAttitude(Memory->Actor.Get(), NewAttitudeAfterReactionComplete, false);
		}
	}
	
	Memory->Actor.Reset();
}
