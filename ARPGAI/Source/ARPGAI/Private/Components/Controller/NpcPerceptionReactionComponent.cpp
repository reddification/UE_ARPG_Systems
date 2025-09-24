#include "Components/Controller/NpcPerceptionReactionComponent.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Tasks/BTTask_CompleteReaction.h"
#include "Data/NpcDTR.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "ReactionEvaluators/NpcReactionEvaluatorBase.h"

// Called when the game starts
void UNpcPerceptionReactionComponent::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if(bNpcComponentInitialized) // this should not happen
	{
		ensure(false);
		return;
	}
	
	bNpcComponentInitialized = true;
#endif

	auto AIController = Cast<AAIController>(GetOwner());
	if (ensure(AIController))
		InitializeAIController(AIController);

	if (auto Pawn = AIController->GetPawn())
		SetPawn(Pawn);
}

void UNpcPerceptionReactionComponent::SetPawn(APawn* InPawn)
{
	if (OwnerNPC.GetObject() == InPawn)
		return;
	
	OwnerNPC.SetObject(InPawn);
	OwnerNPC.SetInterface(Cast<INpc>(InPawn));

	NpcDTRH = OwnerNPC->GetNpcDataTableRowHandle();

	if (const FNpcDTR* NpcDTR = NpcDTRH.GetRow<FNpcDTR>(""))
	{
		BehaviorReactionUtilityBlackboardKeys = NpcDTR->NpcBlackboardDataAsset->ReactionBehaviorUtilityKeys;
		if (!NpcDTR->NpcPerceptionReactionEvaluators.IsEmpty())
		{
			for (const auto* PerceptionReactionEvaluators : NpcDTR->NpcPerceptionReactionEvaluators)
				AddReactionBehaviorEvaluators(PerceptionReactionEvaluators->PerceptionReactionEvaluators);
		}
	}

	OwnerNPC->OnNpcTagsChangedEvent.AddUObject(this, &UNpcPerceptionReactionComponent::OnNpcStateChanged);
	auto GameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	GameMode->NpcWorldStateChangedEvent.AddUObject(this, &UNpcPerceptionReactionComponent::OnWorldStateChanged);
}

void UNpcPerceptionReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto GameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode()))
		GameMode->NpcWorldStateChangedEvent.RemoveAll(this);

	if (IsValid(OwnerNPC.GetObject()))
		OwnerNPC->OnNpcTagsChangedEvent.RemoveAll(this);
	
	Super::EndPlay(EndPlayReason);
}

TArray<FNpcReactionEvaluatorState> UNpcPerceptionReactionComponent::GetPerceptionReactionEvaluators() const
{
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	const auto& CurrentGameTime = NpcGameMode->GetARPGAIGameTime();
	
	TArray<FNpcReactionEvaluatorState> Result;
	for (const auto& BehaviorReactionEvaluator : BehaviorReactionEvaluators)
		for (const auto& ReactionEvaluatorsState : BehaviorReactionEvaluator.Value.NpcReactionEvaluatorStates)
			if (ReactionEvaluatorsState.IsActive(CurrentGameTime))
				Result.Add(ReactionEvaluatorsState);
			
	return Result;
}

const FNpcReactionEvaluatorState* UNpcPerceptionReactionComponent::GetBestBehaviorPerceptionReactionEvaluatorState(
	EReactionBehaviorType ReactionBehaviorType)
{
	const auto BehaviorEvaluators = BehaviorReactionEvaluators.Find(ReactionBehaviorType);
	if (BehaviorEvaluators != nullptr && !BehaviorEvaluators->NpcReactionEvaluatorStates.IsEmpty())
	{
		auto GameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
		const auto& DateTime = GameMode->GetARPGAIGameTime();
		if (ensure(BehaviorEvaluators->NpcReactionEvaluatorStates[0].IsActive(DateTime)))
			return &BehaviorEvaluators->NpcReactionEvaluatorStates[0]; // it is guaranteed that first element of evaluators has the highest utility
	}
	
	return nullptr;
}

void UNpcPerceptionReactionComponent::UpdatePerceptionReactionBehaviorUtility(EReactionBehaviorType ReactionBehaviorType,
	const FGuid& ReactionEvaluatorId, float DeltaUtility)
{
	auto* EvaluatorsWrapper = BehaviorReactionEvaluators.Find(ReactionBehaviorType);
	if (!ensure(EvaluatorsWrapper))
		return;

	for (auto& EvaluatorState : EvaluatorsWrapper->NpcReactionEvaluatorStates)
	{
		if (EvaluatorState.ReactionEvaluator->Id == ReactionEvaluatorId)
		{
			// 30.11.2024 @AK: in theory, multiple perception reaction evaluators can target the same behavior, but with different params
			// In this case we set behavior utility to the highest personal utility of multiple evaluators for the same behavior type
			// but if there's only 1 active evaluator - we can update blackboard utility right away
			if (DeltaUtility > 0.f && !EvaluatorState.bCanAccumulate)
				continue;
			
			const float NewUtility = FMath::Clamp(EvaluatorState.BehaviorUtility + DeltaUtility, 0.f, EvaluatorState.ReactionEvaluator->MaxUtility);
			
			const float MaxUtilityForReactionBehaviorType = EvaluatorsWrapper->NpcReactionEvaluatorStates[0].BehaviorUtility;
			EvaluatorState.BehaviorUtility = NewUtility;
			if (EvaluatorsWrapper->NpcReactionEvaluatorStates.Num() > 1)
				EvaluatorsWrapper->NpcReactionEvaluatorStates.StableSort();

			if (MaxUtilityForReactionBehaviorType != EvaluatorsWrapper->NpcReactionEvaluatorStates[0].BehaviorUtility)
			{
				auto UtilityBlackboardKey = BehaviorReactionUtilityBlackboardKeys.Find(ReactionBehaviorType);
				if (ensure(UtilityBlackboardKey))
					BlackboardComponent->SetValueAsFloat(UtilityBlackboardKey->SelectedKeyName, EvaluatorsWrapper->NpcReactionEvaluatorStates[0].BehaviorUtility);
			}

			break;
		}
	}
}

void UNpcPerceptionReactionComponent::ResetReactionBehaviorUtility(EReactionBehaviorType ReactionBehaviorType, const FGuid& ReactionEvaluatorId)
{
	auto* EvaluatorsWrapper = BehaviorReactionEvaluators.Find(ReactionBehaviorType);
	if (!ensure(EvaluatorsWrapper))
		return;

	for (auto& EvaluatorState : EvaluatorsWrapper->NpcReactionEvaluatorStates)
	{
		if (EvaluatorState.ReactionEvaluator->Id == ReactionEvaluatorId)
		{
			auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
			if (EvaluatorState.ReactionEvaluator->ReactionBehaviorCooldownGameTimeHours > 0.f)
				EvaluatorState.CooldownUntilGameTime = NpcGameMode->GetARPGAIGameTime() + FTimespan::FromHours(EvaluatorState.ReactionEvaluator->ReactionBehaviorCooldownGameTimeHours);
			
			EvaluatorState.BehaviorUtility = 0.f;
			if (EvaluatorsWrapper->NpcReactionEvaluatorStates.Num() > 1)
				EvaluatorsWrapper->NpcReactionEvaluatorStates.StableSort();

			auto UtilityBlackboardKey = BehaviorReactionUtilityBlackboardKeys.Find(ReactionBehaviorType);
			if (ensure(UtilityBlackboardKey))
				BlackboardComponent->SetValueAsFloat(UtilityBlackboardKey->SelectedKeyName, EvaluatorsWrapper->NpcReactionEvaluatorStates[0].BehaviorUtility);
			
			break;
		}
	}
}

void UNpcPerceptionReactionComponent::InitializeAIController(AAIController* AIController)
{
	if (bAIControllerInitialized)
		return;
	
	BlackboardComponent = AIController->GetBlackboardComponent();
	bAIControllerInitialized = true;
}

void UNpcPerceptionReactionComponent::AddReactionBehaviorEvaluators(const UNpcPerceptionReactionEvaluatorsDataAsset* ReactionBehaviorEvaluatorsDataAsset)
{
	AddReactionBehaviorEvaluators(ReactionBehaviorEvaluatorsDataAsset->PerceptionReactionEvaluators);
}

void UNpcPerceptionReactionComponent::AddReactionBehaviorEvaluators(const TArray<UNpcReactionEvaluatorBase*>& ReactionBehaviorEvaluators)
{
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	const FGameplayTagContainer& WorldState = NpcGameMode->GetWorldState();
	const FGameplayTagContainer& CharacterState = OwnerNPC->GetNpcOwnerTags();
	for (const auto& PerceptionReactionEvaluator : ReactionBehaviorEvaluators)
	{
		UObject* EvaluatorMemory = PerceptionReactionEvaluator->CreateMemory(this);

		FNpcReactionEvaluatorState NpcReactionEvaluatorState;
		// 25.12.2024 @AK: I'm not sure it's safe to just copy pointer to an instanced UObject that is in a data asset
		// I assume that at some point, when the data asset is unloaded, the ReactionEvaluator might become nullptr
		// or maybe the data asset, that stores perception reaction behavior evaluators will never be unloaded
		// because its objects are referenced by this component under UPROPERTY
		// TODO consider NpcReactionEvaluatorState.ReactionEvaluator = DuplicateObject(PerceptionReactionEvaluator, this);
		NpcReactionEvaluatorState.ReactionEvaluator = PerceptionReactionEvaluator;
		NpcReactionEvaluatorState.bCanAccumulate = PerceptionReactionEvaluator->CanAccumulateOnlyAtCharacterState.IsEmpty() || PerceptionReactionEvaluator->CanAccumulateOnlyAtCharacterState.Matches(CharacterState);
		NpcReactionEvaluatorState.bWorldStateFilterPasses = PerceptionReactionEvaluator->ActiveAtWorldState.IsEmpty() || PerceptionReactionEvaluator->ActiveAtWorldState.Matches(WorldState);
		NpcReactionEvaluatorState.EvaluatorMemory = EvaluatorMemory;
		
		FNpcReactionEvaluatorStateWrapper& BehaviorReactionEvaluatorsWrapper = BehaviorReactionEvaluators.FindOrAdd(PerceptionReactionEvaluator->ReactionBehaviorType);
		BehaviorReactionEvaluatorsWrapper.NpcReactionEvaluatorStates.Add(NpcReactionEvaluatorState);
	}
}

void UNpcPerceptionReactionComponent::RemoveReactionBehaviorEvaluators(const UNpcPerceptionReactionEvaluatorsDataAsset* ReactionBehaviorEvaluatorsDataAsset)
{
	RemoveReactionBehaviorEvaluators(ReactionBehaviorEvaluatorsDataAsset->PerceptionReactionEvaluators);
}

void UNpcPerceptionReactionComponent::RemoveReactionBehaviorEvaluators(
	const TArray<UNpcReactionEvaluatorBase*>& PerceptionReactionEvaluators)
{
	bool bRemoved = false;
	for (const auto* PerceptionEvaluator : PerceptionReactionEvaluators)
	{
		FNpcReactionEvaluatorStateWrapper* ReactionEvaluatorStateWrapper = BehaviorReactionEvaluators.Find(PerceptionEvaluator->ReactionBehaviorType);
		if (ReactionEvaluatorStateWrapper)
		{
			for (int i = 0; i < ReactionEvaluatorStateWrapper->NpcReactionEvaluatorStates.Num(); i++)
			{
				if (PerceptionEvaluator->Id == ReactionEvaluatorStateWrapper->NpcReactionEvaluatorStates[i].ReactionEvaluator->Id)
				{
					// if it is an active reaction evaluator - reset blackboard utility
					ResetReactionBehaviorUtility(PerceptionEvaluator->ReactionBehaviorType, PerceptionEvaluator->Id);
					ReactionEvaluatorStateWrapper->NpcReactionEvaluatorStates.RemoveAt(i);				
					bRemoved = true;
					break;
				}
			}
		}
	}

	ensure(bRemoved);
}

void UNpcPerceptionReactionComponent::OnWorldStateChanged(const FGameplayTagContainer& NewWorldState)
{
	auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	const auto& CurrentGameTime = NpcGameMode->GetARPGAIGameTime();
	TArray<TTuple<EReactionBehaviorType, FGuid>> EvaluatorsToRemove;
	for (auto& BehaviorEvaluatorStates : BehaviorReactionEvaluators)
	{
		auto& Evaluators = BehaviorEvaluatorStates.Value.NpcReactionEvaluatorStates;
		for (int i = Evaluators.Num() - 1; i >= 0; --i)
		{
			const bool bWasActive = Evaluators[i].IsActive(CurrentGameTime);
			Evaluators[i].bWorldStateFilterPasses = Evaluators[i].ReactionEvaluator->ActiveAtWorldState.IsEmpty() ||
				Evaluators[i].ReactionEvaluator->ActiveAtWorldState.Matches(NewWorldState);
			if (bWasActive && !Evaluators[i].IsActive(CurrentGameTime))
			{
				if (bWasActive && Evaluators[i].ReactionEvaluator->bRemoveOnWorldStateDoesntPassAnymore)
				{
					Evaluators.RemoveAt(i);	
				}
				else
				{
					UpdatePerceptionReactionBehaviorUtility(Evaluators[i].ReactionEvaluator->ReactionBehaviorType, Evaluators[i].ReactionEvaluator->Id,
						-Evaluators[i].BehaviorUtility);	
				}
			}
		}
	}
}

void UNpcPerceptionReactionComponent::OnNpcStateChanged(const FGameplayTagContainer& NewNpcState)
{
	for (auto& BehaviorEvaluatorStates : BehaviorReactionEvaluators)
	{
		for (auto& EvaluatorState : BehaviorEvaluatorStates.Value.NpcReactionEvaluatorStates)
		{
			EvaluatorState.bCanAccumulate = EvaluatorState.ReactionEvaluator->CanAccumulateOnlyAtCharacterState.IsEmpty() ||
				EvaluatorState.ReactionEvaluator->CanAccumulateOnlyAtCharacterState.Matches(NewNpcState);
		}
	}
}
