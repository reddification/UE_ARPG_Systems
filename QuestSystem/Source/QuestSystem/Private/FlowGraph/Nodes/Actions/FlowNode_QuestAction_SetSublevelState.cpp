// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_SetSublevelState.h"

#include "Data/QuestTypes.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/QuestSystemGameMode.h"

EQuestActionExecuteResult UFlowNode_QuestAction_SetSublevelState::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	auto SublevelState = Context.GameMode->SetQuestSublevelState(SublevelId, bLoaded, PostponeIfPlayerHasTags);

	if (bLoaded && SublevelState != EDataLayerRuntimeState::Activated)
	{
		Context.GameMode->QuestDataLayerStateChangedEvent.AddUObject(this, &UFlowNode_QuestAction_SetSublevelState::OnQuestDataLayerStateChanged);
		return EQuestActionExecuteResult::Latent;
	}
	else
		return EQuestActionExecuteResult::Success;
}

void UFlowNode_QuestAction_SetSublevelState::OnQuestDataLayerStateChanged(const FGameplayTag& DataLayerId, bool bNewLoaded)
{
	if (DataLayerId == SublevelId)
	{
		// delaying for 1 frame because i want to make sure that everything has BeginPlay called
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, bNewLoaded]()
		{
			auto QuestGameMode = Cast<IQuestSystemGameMode>(GetWorld()->GetAuthGameMode());
			QuestGameMode->QuestDataLayerStateChangedEvent.RemoveAll(this);
			OnLatentActionFinished(bNewLoaded == bLoaded ? EQuestActionExecuteResult::Success : EQuestActionExecuteResult::Failure);	
		});
	}
}

#if WITH_EDITOR

FString UFlowNode_QuestAction_SetSublevelState::GetQuestActionDescription() const
{
	auto Base = Super::GetQuestActionDescription();

	if (SublevelId.IsValid())
		Base += FString::Printf(TEXT("%s %s"), bLoaded ? TEXT("Load") : TEXT("Unload"), *SublevelId.ToString());
	else
		Base += TEXT("Warning: no sublevel specified");
	
	return Base;
}

EDataValidationResult UFlowNode_QuestAction_SetSublevelState::ValidateNode()
{
	auto Base = Super::ValidateNode();
	if (Base == EDataValidationResult::Invalid)
		return Base;

	return SublevelId.IsValid() ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}

#endif
