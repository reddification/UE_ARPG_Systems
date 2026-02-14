#include "Components/Controller/NpcFlowComponent.h"

#include "FlowAsset.h"
#include "FlowSubsystem.h"
#include "Data/LogChannels.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "FlowGraph/Nodes/FlowNode_NpcGoal.h"

UNpcFlowComponent::UNpcFlowComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
}

void UNpcFlowComponent::InitializeComponent()
{
	Super::InitializeComponent();
	ActivitiesStack.Add(FNpcActivity()); // adding lifecycle activity right from the start. it will always be at zero index	
}

void UNpcFlowComponent::SetDayTime(const FGameplayTag& DayTime)
{
	CurrentDayTime = DayTime;
	
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::SetDayTime %s"), *DayTime.ToString());
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::SetDayTime::Begin"));
	
	if (ActivitiesStack.Num() == 1)
		StartRoutineActivity();
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::SetDayTime::End"));
}

void UNpcFlowComponent::RequestQuestActivity(const FGameplayTag& QuestActivityId)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::RequestQuestActivity %s"), *QuestActivityId.ToString());
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::RequestQuestActivity::Begin"));
	
	auto QuestActivityIdTagName = QuestActivityId.GetTagName();
	const bool bCustomInputExists = GetRootFlowInstance()->TryFindCustomInputNodeByEventName(QuestActivityIdTagName) != nullptr;
	if (!bCustomInputExists)
	{
		ensure(false);
		LogDumpActivitiesStack(TEXT("UNpcFlowComponent::RequestQuestActivity::End"));
		return;
	}
	
	if (!ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid())
		ActivitiesStack.Last().ActiveGoalNode->RequestSuspendGoal();
	
	FNpcActivity NewQuestActivity;
	NewQuestActivity.ActivityTag = QuestActivityId;
	ActivitiesStack.Add(NewQuestActivity);
	TriggerRootFlowCustomInput(QuestActivityIdTagName);
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::RequestQuestActivity::End"));
}

void UNpcFlowComponent::StopQuestActivity(const FGameplayTag& QuestActivityId)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::StopQuestActivity %s"), *QuestActivityId.ToString());
	if (ActivitiesStack.IsEmpty())
		return;

	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::StopQuestActivity::Begin"));
	
	bool bWasLast = false;
	for (int i = ActivitiesStack.Num() - 1; i >= 0; i--)
	{
		if (ActivitiesStack[i].ActivityTag == QuestActivityId)
		{
			ActivitiesStack[i].ActiveGoalNode->RequestExternalAbort();
			bWasLast = i == ActivitiesStack.Num() - 1;
			ActivitiesStack.RemoveAt(i);
			UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::StopQuestActivity: quest activity removed. was %s"),
				bWasLast ? TEXT("last") : TEXT("not last"));
			break;
		}
	}

	if (!bWasLast)
	{
		LogDumpActivitiesStack(TEXT("UNpcFlowComponent::StopQuestActivity::End"));
		return;
	}
	
	if (ActivitiesStack.Num() > 1)
	{
		ActivitiesStack.Last().ActiveGoalNode->RequestResumeGoal();
	}
	else
	{
		if (CurrentDayTime == LifecycleActivitySuspendedAtDayTime)
		{
			ActivitiesStack.Last().ActiveGoalNode->RequestResumeGoal();
		}
		else
		{
			ActivitiesStack.Last().ActiveGoalNode->RequestExternalAbort();
			StartRoutineActivity();
		}
	}
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::StopQuestActivity::End"));
}

void UNpcFlowComponent::PauseGoal()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::pause goal"));
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::PauseGoal::Begin"));
	if (!ActivitiesStack.IsEmpty())
	{
		// idk, it happened to be invalid once on PiE stop
		const auto& Goal = ActivitiesStack.Last().ActiveGoalNode;
		if (Goal.IsValid())
			Goal->RequestSuspendGoal();
	}
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::PauseGoal::End"));
}

void UNpcFlowComponent::ResumeGoal()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::resume goal"));
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::ResumeGoal::Begin"));
	
	if (!ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid())
		ActivitiesStack.Last().ActiveGoalNode->RequestResumeGoal();
	else 
		{ UE_VLOG(GetOwner(), LogARPGAI_Activity, Warning, TEXT("NPC had no goal to resume")); }
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::ResumeGoal::End"));
}

ENpcGoalAdvanceResult UNpcFlowComponent::AdvanceGoal(const FGameplayTagContainer& GoalExecutionResult)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::advance goal"));
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::AdvanceGoal::Begin"));
	
	auto Result = !ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid()
		? ActivitiesStack.Last().ActiveGoalNode->RequestAdvanceGoal(GoalExecutionResult)
		: ENpcGoalAdvanceResult::Failed;
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::AdvanceGoal::End"));
	return Result;
}

void UNpcFlowComponent::OnNpcReachedLocation(const FGameplayTag& LocationIdTag)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::npc reached location"));
	NpcLocationCrossedEvent.ExecuteIfBound(LocationIdTag, true);
}

void UNpcFlowComponent::OnNpcLeftLocation(const FGameplayTag& LocationId)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::npc left location"));
	NpcLocationCrossedEvent.ExecuteIfBound(LocationId, false);
}

void UNpcFlowComponent::SetGoalTagParameter(const FGameplayTag& ParameterId, const FGameplayTag& ParameterValue)
{
	ensure(ParameterValue.IsValid());
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::SetGoalTagParameter::Begin/End"));
	if (GoalTagParameters.Contains(ParameterId))
	{
		UE_VLOG(GetOwner(), LogARPGAI_Activity, Warning, TEXT("UNpcFlowComponent::set goal tag parameter: NPC already has a parameter for %s\nOld value = %s\nNew value = %s"),
			*ParameterId.ToString(), *GoalTagParameters[ParameterId].ToString(), *ParameterValue.ToString());
	}
	
	GoalTagParameters.FindOrAdd(ParameterId) = ParameterValue;
	UE_VLOG(GetOwner(), LogARPGAI_Activity, VeryVerbose, TEXT("UNpcFlowComponent::set goal tag parameter %s = %s"),
		*ParameterId.ToString(), *ParameterValue.ToString());
}

const FGameplayTag& UNpcFlowComponent::GetGoalTagParameter(const FGameplayTag& ParameterId) const
{
	return GoalTagParameters.Contains(ParameterId) ? GoalTagParameters[ParameterId] : FGameplayTag::EmptyTag;
}

void UNpcFlowComponent::RemoveGoalTagParameter(const FGameplayTag& ParameterId)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::remove goal tag parameter %s"), *ParameterId.ToString());
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::RemoveGoalTagParameter::Begin/End"));
	GoalTagParameters.Remove(ParameterId);
}

FEQSParametrizedQueryExecutionRequest* UNpcFlowComponent::GetGoalEQSRequest(const FGameplayTag& EqsId)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::get goal EQS %s"), *EqsId.ToString());
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::GetGoalEQSRequest::Begin/End"));
	if (!ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid())
		return ActivitiesStack.Last().ActiveGoalNode->GetEQSRequest(EqsId);
	
	return nullptr;
}

void UNpcFlowComponent::ExternalCompleteGoal()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::external complete goal"));
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::ExternalCompleteGoal::Begin"));
	
	if (!ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid())
		ActivitiesStack.Last().ActiveGoalNode->RequestExternalAbort();
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::ExternalCompleteGoal::End"));
}

void UNpcFlowComponent::FinishActivity()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::FinishActivity"));
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::FinishActivity::Begin"));
	
	if (ActivitiesStack.Num() > 1)
	{
		auto& LastActivity = ActivitiesStack.Last(); 
		if (LastActivity.ActiveGoalNode.IsValid())
			LastActivity.ActiveGoalNode->RequestExternalAbort();
		
		ActivitiesStack.RemoveAt(ActivitiesStack.Num() - 1);
		if (ensure(ActivitiesStack.Last().ActiveGoalNode.IsValid()))
		{
			ActivitiesStack.Last().ActiveGoalNode->RequestResumeGoal();
		}
	}
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::FinishActivity::End"));
}

void UNpcFlowComponent::StartRoutineActivity()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::start routine activity"));
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::StartRoutineActivity::Begin"));
	TriggerRootFlowCustomInput(CurrentDayTime.GetTagName());
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::StartRoutineActivity::End"));
}

void UNpcFlowComponent::OnGoalStarted(UFlowNode_NpcGoal* NpcGoal)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::on goal %s started [%s]"), *NpcGoal->GetName(), *ActivitiesStack.Last().ActivityTag.ToString());
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::OnGoalStarted::Begin"));

	auto& CurrentActivity = ActivitiesStack.Last();
	CurrentActivity.ActiveGoalNode = NpcGoal;
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::OnGoalStarted::End"));
}

void UNpcFlowComponent::OnGoalEnded(UFlowNode_NpcGoal* NpcGoal)
{
	// it can be not true if the goal failed before even starting 
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::on goal %s ended [%s]"), *NpcGoal->GetName(), *ActivitiesStack.Last().ActivityTag.ToString());
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::OnGoalEnded::Begin"));
	
	if (ActivitiesStack.Last().ActiveGoalNode == NpcGoal)
		ActivitiesStack.Last().ActiveGoalNode = nullptr;
	
	LogDumpActivitiesStack(TEXT("UNpcFlowComponent::OnGoalEnded::End"));
}


void UNpcFlowComponent::LogDumpActivitiesStack(FString Comment)
{
#if !WITH_EDITOR
	return;
#endif
	
	auto OwnerLocal = GetOwner();
	if (!IsValid(OwnerLocal))
		return;
	
	UE_VLOG(OwnerLocal, LogARPGAI_Activity, VeryVerbose, TEXT("Dumping activities stack [%s]"), *Comment);
	for (int i = 0; i < ActivitiesStack.Num(); i++)
	{
		if (ActivitiesStack[i].ActiveGoalNode.IsValid())
		{
			const auto Goal = ActivitiesStack[i].ActiveGoalNode.Get();
			UE_VLOG(OwnerLocal, LogARPGAI_Activity, VeryVerbose, TEXT("[%d] %s\nGoal %s [%s]\nActivation state: %s\nHas finished: %s\nGameplay state: %s"),
				i, *ActivitiesStack[i].ActivityTag.ToString(), *Goal->GetName(), *StaticEnum<ENpcGoalType>()->GetDisplayValueAsText(Goal->GetGoalType()).ToString(),
				*StaticEnum<EFlowNodeState>()->GetDisplayValueAsText(Goal->GetActivationState()).ToString(),
				Goal->HasFinished() ? TEXT("true") : TEXT("false"), *StaticEnum<EGoalState>()->GetDisplayValueAsText(Goal->GetGoalState()).ToString());
		}
		else
		{
			UE_VLOG(OwnerLocal, LogARPGAI_Activity, VeryVerbose, TEXT("[%d] %s; Active goal is null"), i, *ActivitiesStack[i].ActivityTag.ToString());
		}
	}
}
