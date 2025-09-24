// 


#include "FlowGraph/Addons/FlowNodeAddon_DayTimeChange.h"

#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Nodes/FlowNode.h"

UFlowNodeAddon_DayTimeChange::UFlowNodeAddon_DayTimeChange()
{
#if WITH_EDITOR
	for (const auto& OutputTag : DayTimeTagToOutput)
		OutputPins.Add(FFlowPin(OutputTag.Value, EFlowPinType::Exec));
#endif
}

void UFlowNodeAddon_DayTimeChange::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	if (INpcSystemGameMode* GameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode()))
		DayTimeChangedDelegate = GameMode->NpcDayTimeChangedEvent.AddUObject(this, &UFlowNodeAddon_DayTimeChange::OnDayTimeChanged);
}

void UFlowNodeAddon_DayTimeChange::FinishState()
{
	if (DayTimeChangedDelegate.IsValid())
		if (INpcSystemGameMode* GameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode()))
			GameMode->NpcDayTimeChangedEvent.Remove(DayTimeChangedDelegate);

	DayTimeChangedDelegate.Reset();
	Super::FinishState();
}

#if WITH_EDITOR

void UFlowNodeAddon_DayTimeChange::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!PropertyChangedEvent.Property)
	{
		return;
	}

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	const FName MemberPropertyName = PropertyChangedEvent.GetMemberPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UFlowNodeAddon_DayTimeChange, DayTimeTagToOutput) || MemberPropertyName == GET_MEMBER_NAME_CHECKED(UFlowNodeAddon_DayTimeChange, DayTimeTagToOutput))
	{
		OutputPins.Empty();
		for (const auto& OutputTag : DayTimeTagToOutput)
			OutputPins.Add(FFlowPin(OutputTag.Value, EFlowPinType::Exec));

		OnReconstructionRequested.ExecuteIfBound();
	}
}

TArray<FFlowPin> UFlowNodeAddon_DayTimeChange::GetContextOutputs() const
{
	auto ContextOutputs = Super::GetContextOutputs();
	
	for (const auto& OutputTag : DayTimeTagToOutput)
		ContextOutputs.Add(FFlowPin(OutputTag.Value, EFlowPinType::Exec));
	
	return ContextOutputs;
}

#endif

void UFlowNodeAddon_DayTimeChange::OnDayTimeChanged(const FGameplayTag& NewDayTime)
{
	if (DayTimeTagToOutput.Contains(NewDayTime))
		TriggerOutput(DayTimeTagToOutput[NewDayTime], false);
}
