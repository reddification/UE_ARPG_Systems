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
	
	if (ActivitiesStack.Num() == 1)
		StartRoutineActivity();
}

void UNpcFlowComponent::RequestQuestActivity(const FGameplayTag& QuestActivityId)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::RequestQuestActivity %s"), *QuestActivityId.ToString());
	
	auto QuestActivityIdTagName = QuestActivityId.GetTagName();
	const bool bCustomInputExists = GetRootFlowInstance()->TryFindCustomInputNodeByEventName(QuestActivityIdTagName) != nullptr;
	if (!bCustomInputExists)
	{
		ensure(false);
		return;
	}
	
	if (!ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid())
		ActivitiesStack.Last().ActiveGoalNode->RequestSuspendGoal();
	
	FNpcActivity NewQuestActivity;
	NewQuestActivity.ActivityTag = QuestActivityId;
	ActivitiesStack.Add(NewQuestActivity);
	TriggerRootFlowCustomInput(QuestActivityIdTagName);
}

void UNpcFlowComponent::StopQuestActivity(const FGameplayTag& QuestActivityId)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::StopQuestActivity %s"), *QuestActivityId.ToString());

	if (ActivitiesStack.IsEmpty())
		return;

	bool bWasLast = false;
	for (int i = ActivitiesStack.Num() - 1; i >= 0; i--)
	{
		if (ActivitiesStack[i].ActivityTag == QuestActivityId)
		{
			ActivitiesStack[i].ActiveGoalNode->RequestExternalAbort();
			ActivitiesStack.RemoveAt(i);
			bWasLast = i == ActivitiesStack.Num() - 1;
			UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::StopQuestActivity: quest activity removed. was %s"),
				bWasLast ? TEXT("last") : TEXT("not last"));
			break;
		}
	}

	if (!bWasLast)
		return;

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
}

void UNpcFlowComponent::PauseGoal()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::pause goal"));
	if (!ActivitiesStack.IsEmpty())
	{
		// idk, it happened to be invalid once on PiE stop
		const auto& Goal = ActivitiesStack.Last().ActiveGoalNode;
		if (Goal.IsValid())
			Goal->RequestSuspendGoal();
	}
}

void UNpcFlowComponent::ResumeGoal()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::resume goal"));
	if (!ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid())
		ActivitiesStack.Last().ActiveGoalNode->RequestResumeGoal();
}

ENpcGoalAdvanceResult UNpcFlowComponent::AdvanceGoal(const FGameplayTagContainer& GoalExecutionResult)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::advance goal"));

	return !ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid()
		? ActivitiesStack.Last().ActiveGoalNode->RequestAdvanceGoal(GoalExecutionResult)
		: ENpcGoalAdvanceResult::Failed;
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
	GoalTagParameters.Remove(ParameterId);
}

FEQSParametrizedQueryExecutionRequest* UNpcFlowComponent::GetGoalEQSRequest(const FGameplayTag& EqsId)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::get goal EQS %s"), *EqsId.ToString());
	if (!ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid())
		return ActivitiesStack.Last().ActiveGoalNode->GetEQSRequest(EqsId);
	
	return nullptr;
}

void UNpcFlowComponent::ExternalCompleteGoal()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::external complete goal"));
	if (!ActivitiesStack.IsEmpty() && ActivitiesStack.Last().ActiveGoalNode.IsValid())
		ActivitiesStack.Last().ActiveGoalNode->RequestExternalAbort();
}

void UNpcFlowComponent::FinishActivity()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::finish activity"));
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
}

void UNpcFlowComponent::StartRoutineActivity()
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::start routine activity"));
	TriggerRootFlowCustomInput(CurrentDayTime.GetTagName());
}

void UNpcFlowComponent::OnGoalStarted(UFlowNode_NpcGoal* NpcGoal)
{
	// if (!ActivitiesStack.IsEmpty())
	// 	ActivitiesStack.Last().ActiveGoalNode->RequestSuspendGoal();

	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::on goal %s started [%s]"), *NpcGoal->GetName(), *ActivitiesStack.Last().ActivityTag.ToString());
	auto& CurrentActivity = ActivitiesStack.Last();
	// if (CurrentActivity.ActiveGoalNode.IsValid())
	// 	CurrentActivity.ActiveGoalNode->RequestExternalAbort();
	
	CurrentActivity.ActiveGoalNode = NpcGoal;
}

void UNpcFlowComponent::OnGoalEnded(UFlowNode_NpcGoal* NpcGoal)
{
	// it can be not true if the goal failed before even starting 
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("UNpcFlowComponent::on goal %s ended [%s]"), *NpcGoal->GetName(), *ActivitiesStack.Last().ActivityTag.ToString());
	if (ActivitiesStack.Last().ActiveGoalNode == NpcGoal)
		ActivitiesStack.Last().ActiveGoalNode = nullptr;
	
	// if (ensure(!ActivitiesStack.IsEmpty() && NpcGoal == ActivitiesStack.Last().Ac))
	// 	ActivitiesStack.Pop();
	//
	// if (bContinueGoalFromStack && !ActivitiesStack.IsEmpty())
	// 	ActivitiesStack.Last().ActiveGoalNode->RequestResumeGoal();
}
