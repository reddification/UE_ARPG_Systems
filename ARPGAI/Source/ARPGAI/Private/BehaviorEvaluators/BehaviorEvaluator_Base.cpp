#include "BehaviorEvaluators/BehaviorEvaluator_Base.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"

UBehaviorEvaluatorConfig_Base::UBehaviorEvaluatorConfig_Base()
{
	UtilityBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Base, UtilityBBKey));
}

UBlackboardData* UBehaviorEvaluatorConfig_Base::GetBlackboardAsset() const
{
	return !Blackboard.IsNull() ? Blackboard.LoadSynchronous() : nullptr;
}

FBehaviorEvaluator_Base::FBehaviorEvaluator_Base(UBehaviorTreeComponent& OwnerComp, const UConfig* Config)
{
	Blackboard = OwnerComp.GetBlackboardComponent();
	AIController = OwnerComp.GetAIOwner();
	Pawn = AIController->GetPawn();
	PerceptionComponent = AIController->FindComponentByClass<UNpcPerceptionComponent>();
	CombatLogicComponent = GetNpcCombatLogicComponent(OwnerComp);
	NpcComponent = GetNpcComponent(Pawn.Get());
	BaseConfig = Config;
}

void FBehaviorEvaluator_Base::Update(const float DeltaTime)
{
	ensure(DeltaTime < 3.f); // for debug purposes
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("====================\nBehavior evaluator %s begin:"),
		*BaseConfig->BehaviorEvaluatorTag.ToString());
}

void FBehaviorEvaluator_Base::SetState(EBehaviorEvaluatorState NewState)
{
	auto OldState = EvaluatorState;
	EvaluatorState = NewState;
	if (OldState == NewState)
		return;
	
#if WITH_EDITOR
	auto StaticEnumLocal = StaticEnum<EBehaviorEvaluatorState>();
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Log, TEXT("Evaluator %s changed state: %s -> %s"), 
		*BaseConfig->BehaviorEvaluatorTag.ToString(), *StaticEnumLocal->GetDisplayValueAsText(OldState).ToString(),
		*StaticEnumLocal->GetDisplayValueAsText(NewState).ToString());
#endif
	
	if (OldState == EBehaviorEvaluatorState::Activated)
		Cleanup();

	bool bResetUtility = false;
	if (BaseConfig->bResetUtilityOnCeaseRelevant)
		bResetUtility = OldState == EBehaviorEvaluatorState::Relevant && NewState != EBehaviorEvaluatorState::Activated;
	
	if (!bResetUtility && BaseConfig->bResetUtilityOnBecomeRelevant)
		bResetUtility = NewState == EBehaviorEvaluatorState::Relevant && OldState != EBehaviorEvaluatorState::Activated;

	if (bResetUtility)
		Blackboard->SetValueAsFloat(BaseConfig->UtilityBBKey.SelectedKeyName, 0.f);
	
	if (NewState == EBehaviorEvaluatorState::Activated)
		OnActivated();
}

void FBehaviorEvaluator_Base::OnBehaviorFinished(EBehaviorEvaluatorResult ExecutionResult)
{
	switch (ExecutionResult)
	{
		case EBehaviorEvaluatorResult::Success:
			break;
		case EBehaviorEvaluatorResult::Fail:
			break;
		case EBehaviorEvaluatorResult::Abort:
			break;
		case EBehaviorEvaluatorResult::Remove:
			break;
	}
}

float FBehaviorEvaluator_Base::GetMaxUtility() const
{
	return BaseConfig->MaxUtility + ConditionalUtilityEffect;
}

void FBehaviorEvaluator_Base::SetMaxUtility() const
{
	Blackboard->SetValueAsFloat(BaseConfig->UtilityBBKey.SelectedKeyName, GetMaxUtility());
}

void FBehaviorEvaluator_Base::DelayRegression(float NewDelay, bool bAppendToExisting)
{
	bRegressionPostponed = true;
	if (bAppendToExisting && PostponeRegressionUntil > 0.f)
		PostponeRegressionUntil += NewDelay;
	else 
		PostponeRegressionUntil = Pawn->GetWorld()->GetTimeSeconds() + NewDelay;
}

void FBehaviorEvaluator_Base::RequestRegressionFrozen(bool bFrozen)
{
	RegressionFreezeCounter += bFrozen ? 1 : -1;
	if (!ensure(RegressionFreezeCounter >= 0))
		RegressionFreezeCounter = 0;
}

void FBehaviorEvaluator_Base::HandleMessage(const FGameplayTag& MessageTag)
{
	if (CanHandleMessages())
		HandleMessage_Internal(MessageTag);
}

void FBehaviorEvaluator_Base::SetConditionalUtilityEffectActive(const FGameplayTag& ConditionalEffectTag, bool bActive)
{
	if (!BaseConfig->UtilityConditionalOffsets.Contains(ConditionalEffectTag))
		return;

	float Delta = BaseConfig->UtilityConditionalOffsets[ConditionalEffectTag];
	if (bActive)
	{
		if (ActiveUtilityConditions.HasTag(ConditionalEffectTag))
		{
			UE_VLOG(AIController.Get(), LogARPGAI_BE, Warning, TEXT("Attempt to add conditional effect %s multiple times to BE %s"),
				*ConditionalEffectTag.ToString(), *BaseConfig->BehaviorEvaluatorTag.ToString());
			return;
		}
		
		ActiveUtilityConditions.AddTag(ConditionalEffectTag);
	}
	else
	{
		if (!ActiveUtilityConditions.HasTag(ConditionalEffectTag))
		{
			UE_VLOG(AIController.Get(), LogARPGAI_BE, Warning, TEXT("Attempt to remove conditional effect %s to BE %s when it's not applied at all"),
				*ConditionalEffectTag.ToString(), *BaseConfig->BehaviorEvaluatorTag.ToString());
			return;
		}
		
		ActiveUtilityConditions.RemoveTag(ConditionalEffectTag);
		Delta *= -1.f;
	}
	
	ConditionalUtilityEffect += Delta;
	ChangeUtility(Delta);
}

bool FBehaviorEvaluator_Base::CanHandleMessages() const
{
	return EvaluatorState == EBehaviorEvaluatorState::Activated || EvaluatorState == EBehaviorEvaluatorState::Relevant;
}

FDateTime FBehaviorEvaluator_Base::GetGameTime(float GameHoursOffset) const
{
	FDateTime Result = FDateTime();
	if (auto NpcGameMode = Cast<INpcSystemGameMode>(Pawn->GetWorld()->GetAuthGameMode()))
		Result = NpcGameMode->GetARPGAIGameTime();
	
	if (GameHoursOffset != 0.f)
		Result += FTimespan::FromHours(GameHoursOffset);
	
	return Result;
}

bool FBehaviorEvaluator_Base::CanRegress()
{
	if (RegressionFreezeCounter > 0)
	{
		UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Regression is frozen, Freeze count = %d"), RegressionFreezeCounter);
		return false;
	}
	
	if (bRegressionPostponed)
	{
		if (PostponeRegressionUntil <= Pawn->GetWorld()->GetTimeSeconds())
		{
			bRegressionPostponed = false;
			PostponeRegressionUntil = 0.f;
		}
		else
		{
			UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Regression is postponed until = %.2f"), PostponeRegressionUntil);
		}
	}
	
	return !bRegressionPostponed;
}

void FBehaviorEvaluator_Base::InterpolateUtility(float NewDesiredUtility, const float DeltaTime)
{
	const float CurrentUtility = Blackboard->GetValueAsFloat(BaseConfig->UtilityBBKey.SelectedKeyName);
	NewDesiredUtility = FMath::Clamp(NewDesiredUtility, 0.f, BaseConfig->MaxUtility);
	if (FMath::IsNearlyEqual(NewDesiredUtility, CurrentUtility, 0.0001f))
		return;
	
	const float ChangeRate = NewDesiredUtility > CurrentUtility ? GetUtilityAccumulationRate() : GetUtilityDecayRate();
	const float InterpolatedNewUtility = BaseConfig->bInterpConstant 
		? FMath::FInterpConstantTo(CurrentUtility, NewDesiredUtility, DeltaTime, ChangeRate)
		: FMath::FInterpTo(CurrentUtility, NewDesiredUtility, DeltaTime, ChangeRate);
	
	float TrueDelta = InterpolatedNewUtility - CurrentUtility;
	if (TrueDelta != 0.f)
	{
		if (TrueDelta < 0.f && !CanRegress())
		{
			UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("%s delta < 0 but decay is postponed or frozen so not changing utility"), *BaseConfig->BehaviorEvaluatorTag.ToString());
			UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Behavior evaluator %s end\n===================="), *BaseConfig->BehaviorEvaluatorTag.ToString());
			return;		
		}
		
		float FinalNewUtility = FMath::Clamp(InterpolatedNewUtility, 0.f, BaseConfig->MaxUtility);
		Blackboard->SetValueAsFloat(BaseConfig->UtilityBBKey.SelectedKeyName, FinalNewUtility);
		UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("%s new utility = %.2f"), *BaseConfig->BehaviorEvaluatorTag.ToString(), FinalNewUtility);
	}
	
	UE_VLOG(AIController.Get(), LogARPGAI_BE, Verbose, TEXT("Behavior evaluator %s end\n===================="), *BaseConfig->BehaviorEvaluatorTag.ToString());
}

void FBehaviorEvaluator_Base::OnActivated()
{
	NpcComponent->AddBehaviorStack(BaseConfig->BehaviorEvaluatorTag);
	PostponeRegressionUntil = Pawn->GetWorld()->GetTimeSeconds() + BaseConfig->DecayDelayOnActivation;
	bRegressionPostponed = true;
	BehaviorStartTime = Pawn->GetWorld()->GetTimeSeconds();
}

float FBehaviorEvaluator_Base::GetUtilityAccumulationRate() const
{
	ensure(EvaluatorState != EBehaviorEvaluatorState::NotRequested && EvaluatorState != EBehaviorEvaluatorState::Blocked);
	return EvaluatorState == EBehaviorEvaluatorState::Activated ? BaseConfig->ActiveUtilityAccumulationRate : BaseConfig->InactiveUtilityAccumulationRate;
}

float FBehaviorEvaluator_Base::GetUtilityDecayRate() const
{
	return EvaluatorState == EBehaviorEvaluatorState::Activated ? BaseConfig->ActiveUtilityDecayRate : BaseConfig->InactiveUtilityDecayRate;
}

float FBehaviorEvaluator_Base::GetUtilityOffset() const
{
	ensure(EvaluatorState != EBehaviorEvaluatorState::NotRequested && EvaluatorState != EBehaviorEvaluatorState::Blocked);
	return ConditionalUtilityEffect + (EvaluatorState == EBehaviorEvaluatorState::Activated ? BaseConfig->ActiveUtilityOffset : BaseConfig->InactiveUtilityOffset);
}

void FBehaviorEvaluator_Base::ChangeUtility(float Delta)
{
	if (!Blackboard.IsValid() || !BaseConfig.IsValid())
		return;
	
	const float CurrentUtility = Blackboard->GetValueAsFloat(BaseConfig->UtilityBBKey.SelectedKeyName);
	Blackboard->SetValueAsFloat(BaseConfig->UtilityBBKey.SelectedKeyName, FMath::Clamp(CurrentUtility + Delta, 0.f, GetMaxUtility()));
}

void FBehaviorEvaluator_Base::Cleanup()
{
	if (Blackboard.IsValid() && BaseConfig.IsValid() && BaseConfig->bResetUtilityOnDeactivated)
		Blackboard->SetValueAsFloat(BaseConfig->UtilityBBKey.SelectedKeyName, 0.f);
	
	if (NpcComponent.IsValid() && BaseConfig.IsValid())
		NpcComponent->RemoveBehaviorStack(BaseConfig->BehaviorEvaluatorTag);
	
	bRegressionPostponed = false;
	PostponeRegressionUntil = 0.f;
}
