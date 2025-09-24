// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction.h"

#include "Data/QuestRequirements.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "Subsystems/QuestNpcSubsystem.h"
#include "Subsystems/QuestSubsystem.h"

FName UFlowNode_QuestAction::PinOut_Executed("Executed");
FName UFlowNode_QuestAction::PinOut_Failed("Failed");
FName UFlowNode_QuestAction::PinOut_Delayed("Delayed");
FName UFlowNode_QuestAction::PinOut_CannotExecute("Can't execute");

UFlowNode_QuestAction::UFlowNode_QuestAction(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	OutputPins.Append({ PinOut_Executed, PinOut_Failed, PinOut_Delayed, PinOut_CannotExecute });
	if (!ActionId.IsValid())
		ActionId = FGuid::NewGuid();
	
#if WITH_EDITOR
	Category = TEXT("Quests|Actions");
	NodeDisplayStyle = FlowNodeStyle::InOut;
#endif
}

void UFlowNode_QuestAction::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	FQuestSystemContext QSC = UQuestSubsystem::Get(TryGetRootFlowObjectOwner())->GetQuestSystemContext();
	FName OutPinResult = Execute(QSC);
	TriggerOutput(OutPinResult);
	TriggerFirstOutput(OutPinResult != PinOut_Delayed);	
}

#if WITH_EDITOR

FString UFlowNode_QuestAction::GetQuestActionDescription() const
{
	FString Result;
	if (IsDelayed())
	{
		if (GameTimeDelayHours > 0.f)
			Result = FString::Printf(TEXT("Delay for %.2f game hours"), GameTimeDelayHours);
		else
			Result = FString::Printf(TEXT("Delay until %s"), *StartAtNextTimeOfDay.ToString());
	}

	return Result;
}

FText UFlowNode_QuestAction::GetNodeConfigText() const
{
	FString Result = GetQuestActionDescription();
	if (!Result.IsEmpty())
		Result += "\n";

	if (!UserDescription.IsEmpty())
		Result += UserDescription + "\n";
	
	return FText::FromString(Result);
}

#endif

FName UFlowNode_QuestAction::Execute(const FQuestSystemContext& Context)
{
	if (CanExecute(Context))
	{
		if (!IsDelayed())
		{
			EQuestActionExecuteResult Result = ExecuteInternal(Context);
			switch (Result)
			{
				case Success:
					return PinOut_Executed;
				case Failure:
					return PinOut_Failed;
				case Latent:
					return PinOut_Delayed;
				default:
					ensure(false);
					return PinOut_Failed;
			}
		}
		else
		{
			TScriptInterface<IDelayedQuestAction> DelayedQuestActionInterface;
			DelayedQuestActionInterface.SetObject(this);
			DelayedQuestActionInterface.SetInterface(this);
			
			if (GameTimeDelayHours > 0.f)
				Context.QuestSubsystem->DelayAction(ActionId, DelayedQuestActionInterface, GameTimeDelayHours);
			else if (StartAtNextTimeOfDay.IsValid())
				Context.QuestSubsystem->DelayAction(ActionId, DelayedQuestActionInterface, StartAtNextTimeOfDay);
			
			return PinOut_Delayed;
		}
	}
	else
	{
		return PinOut_CannotExecute;
	}
}

bool UFlowNode_QuestAction::CanExecute(const FQuestSystemContext& Context) const
{
	for (const auto& AddOn : AddOns)
	{
		auto PredicateInterface = Cast<IFlowPredicateInterface>(AddOn.Get());
		if (!PredicateInterface->EvaluatePredicate_Implementation())
			return false;
	}
	
	return true;
}

EFlowAddOnAcceptResult UFlowNode_QuestAction::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return IFlowPredicateInterface::ImplementsInterfaceSafe(AddOnTemplate)
		? EFlowAddOnAcceptResult::TentativeAccept
		: EFlowAddOnAcceptResult::Reject;
}

EQuestActionExecuteResult UFlowNode_QuestAction::ExecuteInternal(const FQuestSystemContext& Context)
{
	return EQuestActionExecuteResult::Success;
}

void UFlowNode_QuestAction::OnLatentActionFinished(EQuestActionExecuteResult ExecutionResult)
{
	TriggerOutput(ExecutionResult == EQuestActionExecuteResult::Success ? PinOut_Executed : PinOut_Failed, true);
}

void UFlowNode_QuestAction::StartDelayedAction(const FQuestSystemContext& QuestSystemContext)
{
	if (CanExecute(QuestSystemContext))
	{
		FName PinResult;
		auto ExecutionResult = ExecuteInternal(QuestSystemContext);
		switch (ExecutionResult)
		{
			case Success:
				PinResult = PinOut_Executed;
				break;
			case Failure:
				PinResult = PinOut_Failed;
				break;
			case Latent:
				PinResult = PinOut_Delayed;
				break;
			default:
				ensure(false);
				PinResult = PinOut_Failed;
				break;
		}

		if (PinResult != PinOut_Delayed)
			TriggerOutput(PinResult, true);
	}
	else
	{
		TriggerOutput(PinOut_Failed, true);
	}
}
