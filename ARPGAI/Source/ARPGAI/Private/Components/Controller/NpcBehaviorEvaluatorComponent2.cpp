#include "Components/Controller/NpcBehaviorEvaluatorComponent2.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcDTR.h"
#include "Interfaces/NpcActorTagsInterface.h"

// Sets default values for this component's properties
UNpcBehaviorEvaluatorComponent2::UNpcBehaviorEvaluatorComponent2()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = DefaultComponentTickInterval; 
}

void UNpcBehaviorEvaluatorComponent2::BeginPlay()
{
	Super::BeginPlay();
	if (!BTComponent.IsValid())
	{
		auto AIController = Cast<AAIController>(GetOwner());
		if (ensure(AIController))
			BTComponent = Cast<UBehaviorTreeComponent>(AIController->GetBrainComponent());
	}
}

void UNpcBehaviorEvaluatorComponent2::Initialize(UBehaviorTreeComponent* InBTComponent)
{
	BTComponent = InBTComponent;
	Pawn = InBTComponent->GetAIOwner()->GetPawn();
	ensure(Pawn.IsValid());
	if (auto NpcInterface = Cast<INpcActorTagsInterface>(Pawn.Get()))
	{
		NpcInterface->OnTagsChangedEvent_NPC.AddUObject(this, &UNpcBehaviorEvaluatorComponent2::OnNpcTagsChanged);
		NpcTags = NpcInterface->GetTags_NPC();
	}
}

void UNpcBehaviorEvaluatorComponent2::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                                    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcBehaviorEvaluatorComponent2::TickComponent);
	
	UpdateTemporalRequests();
	const auto WorldTime = GetWorld()->GetTimeSeconds();
	for (auto& EvaluatorWrapper : BehaviorEvaluators)
	{
		if (!EvaluatorWrapper.Value.IsCreated() || !EvaluatorWrapper.Value.Config->bTickable)
			continue;
		
		if (EvaluatorWrapper.Value.LastUpdateAtWorldTime + EvaluatorWrapper.Value.NextUpdateInterval > WorldTime)
			continue;

		auto CurrentState = EvaluatorWrapper.Value.Evaluator->GetState();
		bool bNeedToUpdate = CurrentState == EBehaviorEvaluatorState::Relevant 
			|| CurrentState == EBehaviorEvaluatorState::Activated && EvaluatorWrapper.Value.Config->bUpdateWhenActivated;
		if (bNeedToUpdate)
		{
			EvaluatorWrapper.Value.Evaluator->Update(FMath::Max(DeltaTime, WorldTime - EvaluatorWrapper.Value.LastUpdateAtWorldTime));
			EvaluatorWrapper.Value.LastUpdateAtWorldTime = WorldTime;
			EvaluatorWrapper.Value.NextUpdateInterval = EvaluatorWrapper.Value.Config->TickInterval 
				+ FMath::RandRange( -EvaluatorWrapper.Value.Config->TickIntervalDeviation, EvaluatorWrapper.Value.Config->TickIntervalDeviation);  
		}
	}
	
	if (bPendingUpdateEvaluatorStates)
		UpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::AddEvaluator(const UBehaviorEvaluatorConfig_Base* BehaviorEvaluatorConfig)
{
#if WITH_EDITOR
	for (const auto& EvaluatorWrapper : BehaviorEvaluators)
	{
		if (EvaluatorWrapper.Value.IsCreated())
		{
			if (EvaluatorWrapper.Value.Config->UtilityBBKey.SelectedKeyName == BehaviorEvaluatorConfig->UtilityBBKey.SelectedKeyName)
			{
				ensure(false);
				UE_VLOG(GetOwner(), LogARPGAI_BE, Error, TEXT("Can't add behavior evaluator %s because it reuses already registered evaluator with the same utility key %s [%s]"),
					*BehaviorEvaluatorConfig->BehaviorEvaluatorTag.ToString(), *BehaviorEvaluatorConfig->UtilityBBKey.SelectedKeyName.ToString(),
					*EvaluatorWrapper.Value.Config->BehaviorEvaluatorTag.ToString());
				return;
			}
		}
	}
#endif
	
	FBehaviorEvaluatorWrapper& BEW = BehaviorEvaluators.FindOrAdd(BehaviorEvaluatorConfig->BehaviorEvaluatorTag);
	if (!ensure(!BEW.IsCreated()))
	{
		UE_VLOG_UELOG(GetOwner(), LogARPGAI_BE, Error, TEXT("Attempt to add an evaluator when one already exist [%s]"), 
			*BehaviorEvaluatorConfig->BehaviorEvaluatorTag.ToString())
		return;
	}
	
	BEW.Evaluator = BehaviorEvaluatorConfig->CreateEvaluator(BTComponent.Get());
	BEW.Config = BehaviorEvaluatorConfig;
	BEW.bCharacterStateComplies = BehaviorEvaluatorConfig->RelevancyOwnerRequirements.IsEmpty() 
		? true 
		: BEW.Config->RelevancyOwnerRequirements.Matches(NpcTags);
	
	RequestUpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::RemoveEvaluator(const FGameplayTag& EvaluatorId)
{
	if (auto EvaluatorWrapper = BehaviorEvaluators.Find(EvaluatorId))
	{
		if (EvaluatorWrapper->IsCreated())
		{
			if (EvaluatorWrapper->Evaluator->GetState() == EBehaviorEvaluatorState::Activated)
				DeactivateBehavior(EvaluatorId, EBehaviorEvaluatorResult::Remove);
			else 
				SetEvaluatorState(*EvaluatorWrapper, EBehaviorEvaluatorState::NotRequested);
		}		
			
		if (EvaluatorWrapper->IsRequestStateClean(GetWorld()->GetTimeSeconds()))
			BehaviorEvaluators.Remove(EvaluatorId);
		else if (EvaluatorWrapper->IsCreated())
			EvaluatorWrapper->Purge();
			
		UpdateComponentTick();
	}
}

void UNpcBehaviorEvaluatorComponent2::ActivateBehavior(const FGameplayTag& EvaluatorId)
{
	UE_VLOG(GetOwner(), LogARPGAI_BE, Log, TEXT("Request activate behavior %s"), *EvaluatorId.ToString());
	auto* BEW = BehaviorEvaluators.Find(EvaluatorId);
	if (!ensure(BEW != nullptr && BEW->IsCreated()))
	{
		UE_VLOG_UELOG(GetOwner(), LogARPGAI_BE, Error, TEXT("Attempt to activate non-existing evaluator %s"), *EvaluatorId.ToString());
		return;
	}
	
	auto WorldTime = GetWorld()->GetTimeSeconds();
	if (!ensure(BEW->GetDesiredState(WorldTime, StateRequestPriority) == EBehaviorEvaluatorState::Relevant))
	{
		UE_VLOG_UELOG(GetOwner(), LogARPGAI_BE, Error, TEXT("Attempt to activate non-relevant behavior %s"), *EvaluatorId.ToString());
		return;
	}
	
	SetEvaluatorState(*BEW, EBehaviorEvaluatorState::Activated);
	ActiveBehaviorId = EvaluatorId;
	
	UpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::DeactivateBehavior(const FGameplayTag& EvaluatorId, EBehaviorEvaluatorResult Result)
{
	UE_VLOG(GetOwner(), LogARPGAI_BE, Log, TEXT("Request deactivate behavior %s"), *EvaluatorId.ToString());
	auto* BEW = BehaviorEvaluators.Find(EvaluatorId);
	if (BEW == nullptr)
	{
		ensure(false);
		UE_VLOG_UELOG(GetOwner(), LogARPGAI_BE, Error, TEXT("Attempt to deactivate non-existing behavior %s"), *EvaluatorId.ToString());
		return;
	}
	
	auto WorldTime = GetWorld()->GetTimeSeconds();
	if (BEW->Evaluator->GetState() == EBehaviorEvaluatorState::Activated)
	{
		EBehaviorEvaluatorState DesiredState = BEW->GetDesiredState(WorldTime, StateRequestPriority);
		if (DesiredState == EBehaviorEvaluatorState::Activated)
			DesiredState = EBehaviorEvaluatorState::Relevant;
	
		// OnBehaviorFinished must be called before setting new state
		BEW->Evaluator->OnBehaviorFinished(Result);
		SetEvaluatorState(*BEW, DesiredState);
	}
	else
	{
		UE_VLOG_UELOG(GetOwner(), LogARPGAI_BE, Warning, TEXT("Attempt to deactivate non active evaluator %s"), *EvaluatorId.ToString());
		return;
	}
	
	UpdateEvaluatorsStates();
}

bool UNpcBehaviorEvaluatorComponent2::SetMaxUtility(const FGameplayTag& BehaviorTag)
{
	auto BehaviorEvaluator = BehaviorEvaluators.Find(BehaviorTag);
	if (BehaviorEvaluator == nullptr || !BehaviorEvaluator->IsCreated())
		return false;
	
	BehaviorEvaluator->Evaluator->SetMaxUtility();
	return true;
}

bool UNpcBehaviorEvaluatorComponent2::DelayRegression(const FGameplayTag& BehaviorTag, float Delay, bool bAppendToExisting)
{
	if (auto BEW = BehaviorEvaluators.Find(BehaviorTag))
	{
		if (BEW->IsAccumulating())
		{
			BEW->Evaluator->DelayRegression(Delay, bAppendToExisting); 
			return true;
		}
	} 
	
	return false;
}

bool UNpcBehaviorEvaluatorComponent2::RequestFreezeRegression(const FGameplayTag& EvaluatorId, bool bFrozen)
{
	if (auto BEW = BehaviorEvaluators.Find(EvaluatorId))
	{
		if (BEW->CanFreezeRegression())
		{
			BEW->Evaluator->RequestRegressionFrozen(bFrozen);
			return true;
		}
	}
	
	return false;
}

void UNpcBehaviorEvaluatorComponent2::HandleMessage(const FGameplayTag& EvaluatorTag, const FGameplayTag& Message)
{
	auto BEW = BehaviorEvaluators.Find(EvaluatorTag);
	if (BEW && BEW->IsCreated())
		BEW->Evaluator->HandleMessage(Message);
}

void UNpcBehaviorEvaluatorComponent2::BroadcastMessage(const FGameplayTag& Message)
{
	for (auto& EvaluatorWrapper : BehaviorEvaluators)
		if (EvaluatorWrapper.Value.IsCreated())
			EvaluatorWrapper.Value.Evaluator->HandleMessage(Message);
}

void UNpcBehaviorEvaluatorComponent2::HandleInteractionStateChanged(AActor* InteractionActor,
                                                                    const FGameplayTagContainer& InteractionTags, bool bActive)
{
	for (auto& BEW : BehaviorEvaluators)
		if (BEW.Value.IsCreated())
			BEW.Value.Evaluator->SetConditionalUtilityEffectActive(AIGameplayTags::BehaviorEvaluation_ConditionalUtility_Interaction, bActive);
}

void UNpcBehaviorEvaluatorComponent2::RequestEvaluatorsRelevant(const FGameplayTagContainer& EvaluatorTags, bool bActive, const FName& SourceId)
{
	if (bActive)
	{
		for (const auto& ChangedEvaluator : EvaluatorTags)
		{
			auto& BEW = BehaviorEvaluators.FindOrAdd(ChangedEvaluator);
			BEW.AddStateRequest(SourceId, EBehaviorEvaluatorState::Relevant);
		}
	}
	else
	{
		for (const auto& ChangedEvaluator : EvaluatorTags)
			if (auto BEW = BehaviorEvaluators.Find(ChangedEvaluator))
				BEW->RemoveStateRequest(SourceId);
	}
	
	RequestUpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::RequestEvaluatorsBlocked(const FGameplayTagContainer& EvaluatorTags, bool bActive, const FName& SourceId)
{
	if (bActive)
	{
		for (const auto& ChangedEvaluator : EvaluatorTags)
		{
			auto& BEW = BehaviorEvaluators.FindOrAdd(ChangedEvaluator);
			BEW.AddStateRequest(SourceId, EBehaviorEvaluatorState::Blocked);
		}
	}
	else
	{
		for (const auto& ChangedEvaluator : EvaluatorTags)
			if (auto BEW = BehaviorEvaluators.Find(ChangedEvaluator))
				BEW->RemoveStateRequest(SourceId);
	}

	RequestUpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::RequestEvaluatorRelevant(const FGameplayTag& EvaluatorTag, bool bActive,
                                                               const FName& SourceId)
{
	Super::RequestEvaluatorRelevant(EvaluatorTag, bActive, SourceId);
	if (auto BEW = BehaviorEvaluators.Find(EvaluatorTag))
	{
		if (bActive)
			BEW->AddStateRequest(SourceId, EBehaviorEvaluatorState::Relevant);
		else
			BEW->RemoveStateRequest(SourceId);
	}
	
	RequestUpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::RequestEvaluatorBlocked(const FGameplayTag& EvaluatorTag, bool bActive,
                                                              const FName& SourceId)
{
	Super::RequestEvaluatorBlocked(EvaluatorTag, bActive, SourceId);
	const auto WorldTime = GetWorld()->GetTimeSeconds();
	if (auto BEW = BehaviorEvaluators.Find(EvaluatorTag))
	{
		if (bActive)
		{
			BEW->AddStateRequest(SourceId, EBehaviorEvaluatorState::Blocked);
			UE_VLOG(GetOwner(), LogARPGAI_BE, Verbose, TEXT("Blocking behavior %s by %s indefinitely. Block count = %d"),
				*EvaluatorTag.ToString(), *SourceId.ToString(), BEW->GetBlockRequests(WorldTime));
		}
		else
		{
			BEW->RemoveStateRequest(SourceId);
			UE_VLOG(GetOwner(), LogARPGAI_BE, Verbose, TEXT("Unblocked behavior %s by %s. Block count = %d"),
				*EvaluatorTag.ToString(), *SourceId.ToString(), BEW->GetBlockRequests(WorldTime));
		}
	}
	
	RequestUpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::RequestEvaluatorRelevant(const FGameplayTag& EvaluatorTag, float Duration, const FName& SourceId)
{
	auto& BEW = BehaviorEvaluators.FindOrAdd(EvaluatorTag);
	BEW.AddStateRequest(SourceId, EBehaviorEvaluatorState::Relevant, GetWorld()->GetTimeSeconds() + Duration);
	RequestUpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::RequestEvaluatorBlocked(const FGameplayTag& EvaluatorTag, float Duration, const FName& SourceId)
{
	auto& BEW = BehaviorEvaluators.FindOrAdd(EvaluatorTag);
	BEW.AddStateRequest(SourceId, EBehaviorEvaluatorState::Blocked, GetWorld()->GetTimeSeconds() + Duration);
	RequestUpdateEvaluatorsStates();
}

bool UNpcBehaviorEvaluatorComponent2::SetBehaviorEvaluatorCooldown(const FGameplayTag& EvaluatorTag, float Cooldown, const FName& SourceId)
{
	RequestEvaluatorBlocked(EvaluatorTag, Cooldown, SourceId);
	return true;
}

void UNpcBehaviorEvaluatorComponent2::UpdateTemporalRequests()
{
	bool bNeedsUpdate = false;
	const auto CurrentTime = GetWorld()->GetTimeSeconds();
	for (auto& BehaviorEvaluator : BehaviorEvaluators)
	{
		auto UpdatedRequests = BehaviorEvaluator.Value.UpdateTemporalRequests(CurrentTime);
		bNeedsUpdate |= UpdatedRequests > 0;
	}
	
	if (bNeedsUpdate)
		UpdateEvaluatorsStates();
}

void UNpcBehaviorEvaluatorComponent2::UpdateComponentTick()
{
	if (bPendingUpdateEvaluatorStates)
	{
		if (!IsComponentTickEnabled())
			SetComponentTickEnabled(true);
		
		return;
	}
	
	bool bWantToTick = false;
	if (Pawn.IsValid())
	{
		const auto WorldTime = GetWorld()->GetTimeSeconds();
		for (const auto& Evaluator : BehaviorEvaluators)
		{
			bWantToTick = Evaluator.Value.IsCreated() && Evaluator.Value.GetDesiredState(WorldTime, StateRequestPriority) != EBehaviorEvaluatorState::NotRequested;
			if (bWantToTick)
				break;
		}
	}	
	
	if (IsComponentTickEnabled() != bWantToTick)
		SetComponentTickEnabled(bWantToTick);
}

void UNpcBehaviorEvaluatorComponent2::RequestUpdateEvaluatorsStates()
{
	bPendingUpdateEvaluatorStates = true;
	if (!IsComponentTickEnabled())
		SetComponentTickEnabled(true);
	
	SetComponentTickInterval(0.f);
}

void UNpcBehaviorEvaluatorComponent2::UpdateEvaluatorsStates()
{
	if (bUpdatingEvaluatorsStates)
	{
		UE_VLOG(GetOwner(), LogARPGAI_BE, Warning, TEXT("Well, shit, UNpcBehaviorEvaluatorComponent2::UpdateEvaluatorsStates got called recursively"));
		UpdateEvaluatorsStates();
		return;
	}
	
	TGuardValue<bool> RecursionGuard(bUpdatingEvaluatorsStates, true);
	bPendingUpdateEvaluatorStates = false;
	auto WorldTime = GetWorld()->GetTimeSeconds();
	bool bNeedToUpdateTick = false;
	TArray<FGameplayTag, TInlineAllocator<4>> ToRemove;
	for (auto& EvaluatorWrapper : BehaviorEvaluators)
	{
		if (!EvaluatorWrapper.Value.IsCreated())
		{
			if (EvaluatorWrapper.Value.IsRequestStateClean(WorldTime))
				ToRemove.Add(EvaluatorWrapper.Key);
			
			continue;
		}	
		
		auto CurrentState = EvaluatorWrapper.Value.Evaluator->GetState();
		auto DesiredState = EvaluatorWrapper.Value.GetDesiredState(WorldTime, StateRequestPriority);
		if (CurrentState == DesiredState)
			continue;
		
		bNeedToUpdateTick = true;
		SetEvaluatorState(EvaluatorWrapper.Value, DesiredState);
	}
	
	bNeedToUpdateTick |= !ToRemove.IsEmpty();
	for (const auto& RemovedEvaluator : ToRemove)
		BehaviorEvaluators.Remove(RemovedEvaluator);

	if (!bPendingUpdateEvaluatorStates)
		SetComponentTickInterval(DefaultComponentTickInterval);	
	
	if (bNeedToUpdateTick)
		UpdateComponentTick();
}

void UNpcBehaviorEvaluatorComponent2::SetEvaluatorState(FBehaviorEvaluatorWrapper& EvaluatorWrapper,
	EBehaviorEvaluatorState NewState)
{
	auto PreviousState = EvaluatorWrapper.Evaluator->GetState();
	if (PreviousState == NewState)
	{
		ensure(false);
		return;
	}
	
	if (auto PreviousStateEffects = EvaluatorWrapper.Config->StateEffects.Find(PreviousState))
		for (const auto& EffectIS : PreviousStateEffects->Effects)
			if (ensure(EffectIS.IsValid()))
				EffectIS.Get<FBehaviorEvaluatorStateEffect>().Rollback(*this, EvaluatorWrapper.Config->BehaviorEvaluatorTag);
	
	EvaluatorWrapper.Evaluator->SetState(NewState);
	
	if (auto NewStateEffects = EvaluatorWrapper.Config->StateEffects.Find(NewState))
		for (const auto& EffectIS : NewStateEffects->Effects)
			if (ensure(EffectIS.IsValid()))
				EffectIS.Get<FBehaviorEvaluatorStateEffect>().Apply(*this, EvaluatorWrapper.Config->BehaviorEvaluatorTag);
	
	bool bNeedToResetNextUpdateTime = EvaluatorWrapper.Config->bTickable 
		&& (NewState == EBehaviorEvaluatorState::Relevant || NewState == EBehaviorEvaluatorState::Activated) 
		&& (PreviousState == EBehaviorEvaluatorState::NotRequested 
			|| PreviousState == EBehaviorEvaluatorState::Blocked 
			|| PreviousState == EBehaviorEvaluatorState::Activated && !EvaluatorWrapper.Config->bUpdateWhenActivated);
	
	if (bNeedToResetNextUpdateTime)
		EvaluatorWrapper.LastUpdateAtWorldTime = GetWorld()->GetTimeSeconds();
}

void UNpcBehaviorEvaluatorComponent2::OnNpcTagsChanged(AActor* Npc, const FGameplayTagContainer& NewTags)
{
	NpcTags = NewTags;
	bool bRequiresUpdate = false;
	auto WorldTime = GetWorld()->GetTimeSeconds();
	for (auto& EvaluatorWrapper : BehaviorEvaluators)
	{
		if (!EvaluatorWrapper.Value.IsCreated() || EvaluatorWrapper.Value.Config->RelevancyOwnerRequirements.IsEmpty())
			continue;
		
		bool bPreviouslyComplied = EvaluatorWrapper.Value.bCharacterStateComplies;
		auto PreviousState = EvaluatorWrapper.Value.Evaluator->GetState();
		EvaluatorWrapper.Value.bCharacterStateComplies = EvaluatorWrapper.Value.Config->RelevancyOwnerRequirements.Matches(NewTags);
		if (bPreviouslyComplied != EvaluatorWrapper.Value.bCharacterStateComplies)
		{
			auto NewDesiredState = EvaluatorWrapper.Value.GetDesiredState(WorldTime, StateRequestPriority);
			bRequiresUpdate |= PreviousState != NewDesiredState;
		}
	}
	
	if (bRequiresUpdate)
		UpdateEvaluatorsStates();
}

EBehaviorEvaluatorState UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::GetDesiredState(const double CurrentWorldTime,
	EBehaviorEvaluatorStateRequestPriority StateRequestPriority) const
{
	if (!bCharacterStateComplies || StateRequests.IsEmpty())
		return EBehaviorEvaluatorState::NotRequested;
	
	if (StateRequestPriority == EBehaviorEvaluatorStateRequestPriority::Block)
	{
		bool bHasRelevantRequest = false;
		for (int i = StateRequests.Num() - 1; i >= 0; i++)
		{
			bool bStale = StateRequests[i].IsStale(CurrentWorldTime);
			if (bStale)
				continue;
			
			auto RequestedState = StateRequests[i].State;
			if (const bool bBlocked = RequestedState == EBehaviorEvaluatorState::Blocked)
				return EBehaviorEvaluatorState::Blocked;

			bHasRelevantRequest |= RequestedState == EBehaviorEvaluatorState::Relevant;
		}
		
		return bHasRelevantRequest 
			? Evaluator->GetState() == EBehaviorEvaluatorState::Activated 
				? EBehaviorEvaluatorState::Activated 
				: EBehaviorEvaluatorState::Relevant
			: EBehaviorEvaluatorState::NotRequested;
	}
	else if (StateRequestPriority == EBehaviorEvaluatorStateRequestPriority::Latest)
	{
		for (int i = StateRequests.Num() - 1; i >= 0; --i)
		{
			if (!StateRequests[i].IsStale(CurrentWorldTime))
			{
				if (StateRequests[i].State == EBehaviorEvaluatorState::Blocked)
					return EBehaviorEvaluatorState::Blocked;
				
				if (ensure(StateRequests[i].State == EBehaviorEvaluatorState::Relevant))
				{
					return Evaluator->GetState() == EBehaviorEvaluatorState::Activated 
						? EBehaviorEvaluatorState::Activated  
						: EBehaviorEvaluatorState::Relevant;
				}
				
				break;
			}
		}
	}
	
	return EBehaviorEvaluatorState::NotRequested;
}

bool UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::IsRequestStateClean(const double WorldTime) const
{
	if (StateRequests.IsEmpty())
		return true;
	
	for (int i = StateRequests.Num() - 1; i >= 0; --i)
		if (!StateRequests[i].IsStale(WorldTime))
			return false;

	return true;
}

bool UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::IsAccumulating() const
{
	return IsCreated() && Config->bTickable &&
		(Evaluator->GetState() == EBehaviorEvaluatorState::Relevant 
			|| Evaluator->GetState() == EBehaviorEvaluatorState::Activated && Config->bUpdateWhenActivated);
}

void UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::OnStateRequested(const FName& SourceId, EBehaviorEvaluatorState State)
{
	ensure(State == EBehaviorEvaluatorState::Blocked || State == EBehaviorEvaluatorState::Relevant);
#if WITH_EDITOR
	for (int i = 0; i < StateRequests.Num() - 1; i++)
	{
		if (StateRequests[i].Id == SourceId)
		{
			ensure(false);
			int shit = 1;
		}
	}
#endif	
}

void UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::AddStateRequest(const FName& SourceId, EBehaviorEvaluatorState State)
{
	StateRequests.Add(FStateRequest(SourceId, State));
	OnStateRequested(SourceId, State);
}

void UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::AddStateRequest(const FName& SourceId,
	EBehaviorEvaluatorState State, double UntilWorldTime)
{
	StateRequests.Add(FStateRequest(SourceId, State, UntilWorldTime));
	OnStateRequested(SourceId, State);
}

void UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::RemoveStateRequest(const FName& SourceId)
{
	// 30 May 2026 (aki):
	// it can happen that no requests were removed. for example, if request was temporal and was cleaned before this function was called
	for (int i = StateRequests.Num() - 1; i >= 0; --i)
	{
		if (StateRequests[i].Id == SourceId)
		{
			StateRequests.RemoveAt(i);
			return;
		}
	}
}

int32 UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::GetValidRequestsCount(const double WorldTime,
	EBehaviorEvaluatorState RequestType) const
{
	int32 Result = 0;
	for (int i = 0; i < StateRequests.Num(); ++i)
		if (!StateRequests[i].IsStale(WorldTime) && StateRequests[i].State == RequestType)
			Result++;
	
	return Result;
}

uint8 UNpcBehaviorEvaluatorComponent2::FBehaviorEvaluatorWrapper::UpdateTemporalRequests(double CurrentTime)
{
	uint8 Result = 0;
	for (int i = StateRequests.Num() - 1; i >= 0; --i)
	{
		if (StateRequests[i].IsStale(CurrentTime))
		{
			StateRequests.RemoveAt(i);
			Result++;
		}
	}
	
	return Result;
}
