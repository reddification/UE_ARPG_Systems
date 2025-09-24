// 


#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Data/NpcDTR.h"
#include "Interfaces/BehaviorEvaluator.h"


// Sets default values for this component's properties
UNpcBehaviorEvaluatorComponent::UNpcBehaviorEvaluatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.15f; 
}

void UNpcBehaviorEvaluatorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	auto PendingUnblockEvaluatorsCopy = PendingUnblockEvaluators;
	for (auto& Evaluator : PendingUnblockEvaluatorsCopy)
	{
		PendingUnblockEvaluators[Evaluator.Key] -= DeltaTime;
		if (PendingUnblockEvaluators[Evaluator.Key] <= 0.f)
		{
			PendingUnblockEvaluators.Remove(Evaluator.Key);
			RequestEvaluatorsBlocked(Evaluator.Key.GetSingleTagContainer(), false);
		}
	}

	auto PendingRemoveEvaluatorsActivationsCopy = PendingRemoveEvaluatorsActivations;
	for (auto& Evaluator : PendingRemoveEvaluatorsActivationsCopy)
	{
		PendingRemoveEvaluatorsActivations[Evaluator.Key] -= DeltaTime;
		if (PendingRemoveEvaluatorsActivations[Evaluator.Key] <= 0.f)
		{
			PendingRemoveEvaluatorsActivations.Remove(Evaluator.Key);
			RequestEvaluatorsActive(Evaluator.Key.GetSingleTagContainer(), false);
		}
	}

	if (PendingUnblockEvaluators.IsEmpty() && PendingRemoveEvaluatorsActivations.IsEmpty())
		SetComponentTickEnabled(false);
}

void UNpcBehaviorEvaluatorComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!BTComponent.IsValid())
	{
		auto AIController = Cast<AAIController>(GetOwner());
		BTComponent = Cast<UBehaviorTreeComponent>(AIController->GetBrainComponent());
	}
}

void UNpcBehaviorEvaluatorComponent::RegisterBehaviorEvaluator(const TScriptInterface<IBehaviorEvaluator>& BehaviorEvaluator,
                                                               const FGameplayTag& EvaluatorId)
{
	BehaviorEvaluators.Add(EvaluatorId, BehaviorEvaluator);
}

void UNpcBehaviorEvaluatorComponent::UnregisterBehaviorEvaluator(const FGameplayTag& EvaluatorId)
{
	BehaviorEvaluators.Remove(EvaluatorId);
}

void UNpcBehaviorEvaluatorComponent::InitiateBehaviorState(const FGameplayTag& EvaluatorId)
{
	if (const TScriptInterface<IBehaviorEvaluator>* BE = BehaviorEvaluators.Find(EvaluatorId))
		if (IsValid(BE->GetObject()))
			BE->GetInterface()->InitiateBehaviorState(BTComponent.Get());
}

void UNpcBehaviorEvaluatorComponent::FinalizeBehaviorState(const FGameplayTag& EvaluatorId)
{
	if (const TScriptInterface<IBehaviorEvaluator>* BE = BehaviorEvaluators.Find(EvaluatorId))
		if (IsValid(BE->GetObject()))
			BE->GetInterface()->FinalizeBehaviorState(BTComponent.Get());
}

void UNpcBehaviorEvaluatorComponent::RequestEvaluatorsActive(const FGameplayTagContainer& EvaluatorTags, bool bActive)
{
	if (bActive)
	{
		for (const auto& ChangedEvaluator : EvaluatorTags)
		{
			int& ActiveEvaluatorCount = RequestedActiveEvaluators.FindOrAdd(ChangedEvaluator);
			ActiveEvaluatorCount++;
		}
	}
	else
	{
		for (const auto& ChangedEvaluator : EvaluatorTags)
		{
			if (int* ActiveEvaluatorCount = RequestedActiveEvaluators.Find(ChangedEvaluator))
			{
				(*ActiveEvaluatorCount)--;
				if (*ActiveEvaluatorCount == 0)
					RequestedActiveEvaluators.Remove(ChangedEvaluator);
			}
		}
	}

	UpdateActiveEvaluators();
}

void UNpcBehaviorEvaluatorComponent::RequestEvaluatorsBlocked(const FGameplayTagContainer& EvaluatorTags, bool bActive)
{
	if (bActive)
	{
		for (const auto& ChangedEvaluator : EvaluatorTags)
		{
			int& BlockedEvaluatorCount = RequestedBlockedEvaluators.FindOrAdd(ChangedEvaluator);
			BlockedEvaluatorCount++;
		}
	}
	else
	{
		for (const auto& ChangedEvaluator : EvaluatorTags)
		{
			if (int* BlockedEvaluatorCount = RequestedBlockedEvaluators.Find(ChangedEvaluator))
			{
				(*BlockedEvaluatorCount)--;
				if (*BlockedEvaluatorCount <= 0)
					RequestedBlockedEvaluators.Remove(ChangedEvaluator);
			}
		}
	}

	UpdateActiveEvaluators();
}

void UNpcBehaviorEvaluatorComponent::RequestEvaluatorActive(const FGameplayTag& EvaluatorTag, float Duration)
{
	int& ActiveCount = RequestedActiveEvaluators.FindOrAdd(EvaluatorTag);
	++ActiveCount;
	
	float& RemainingActivationTime = PendingRemoveEvaluatorsActivations.FindOrAdd(EvaluatorTag);
	RemainingActivationTime = Duration;
	SetComponentTickEnabled(true);
}

void UNpcBehaviorEvaluatorComponent::RequestEvaluatorBlocked(const FGameplayTag& EvaluatorTag, float Duration)
{
	int& BlockCount = RequestedBlockedEvaluators.FindOrAdd(EvaluatorTag);
	++BlockCount;
	
	float& RemainingBlockTime = PendingUnblockEvaluators.FindOrAdd(EvaluatorTag);
	RemainingBlockTime += Duration;
	SetComponentTickEnabled(true);
}

void UNpcBehaviorEvaluatorComponent::Initialize(UBehaviorTreeComponent* InBTComponent, const FNpcDTR* NpcDTR)
{
	BTComponent = InBTComponent;
	BlackboardKeys = NpcDTR->NpcBlackboardDataAsset;
}

void UNpcBehaviorEvaluatorComponent::UpdateActiveEvaluators()
{
	auto Blackboard = BTComponent->GetBlackboardComponent();
	auto CurrentlyActiveEvaluators = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->ActiveBehaviorEvaluatorsTagsBBKey.SelectedKeyName);
	FGameplayTagContainer ActiveEvaluators;
	for (const auto& ActiveEvaluator : RequestedActiveEvaluators)
		if (!RequestedBlockedEvaluators.Contains(ActiveEvaluator.Key))
			ActiveEvaluators.AddTagFast(ActiveEvaluator.Key);

	if (CurrentlyActiveEvaluators != ActiveEvaluators)
		Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->ActiveBehaviorEvaluatorsTagsBBKey.SelectedKeyName, ActiveEvaluators);
}