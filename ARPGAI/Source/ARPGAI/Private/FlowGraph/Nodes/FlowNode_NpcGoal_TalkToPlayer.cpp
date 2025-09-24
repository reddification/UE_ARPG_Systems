// 


#include "FlowGraph/Nodes/FlowNode_NpcGoal_TalkToPlayer.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "StructUtils/StructView.h"

ENpcGoalStartResult UFlowNode_NpcGoal_TalkToPlayer::Start()
{
	auto Result = Super::Start();
	if (Parameters.bGoToPlayerDirectly)
	{
		auto PlayerCharacter = UGameplayStatics::GetPlayerCharacter(NpcPawn.Get(), 0);
		BlackboardComponent->SetValueAsObject(BlackboardKeys->ConversationPartnerBBKey.SelectedKeyName, PlayerCharacter);
		BlackboardComponent->ClearValue(BlackboardKeys->EqsToRunBBKey.SelectedKeyName);
	}
	else
	{
		BlackboardComponent->ClearValue(BlackboardKeys->ConversationPartnerBBKey.SelectedKeyName);
		BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, Parameters.PlayerSearchEQS);
	}
	
	return Result;
}

#if WITH_EDITOR

FString UFlowNode_NpcGoal_TalkToPlayer::GetGoalDescription() const
{
	FString Result = Super::GetGoalDescription();
	if (Parameters.OptionalDialogueId.IsValid())
		Result += FString::Printf(TEXT("\nOptional Dialogue: %s\n"), *Parameters.OptionalDialogueId.ToString());
	else
		Result += TEXT("\nStart default dialogue\n");

	if (Parameters.bGoToPlayerDirectly)
		Result += TEXT("Go to player directly\n");
	else if (!Parameters.PlayerSearchEQS)
		Result += FString::Printf(TEXT("Search for player in EQS: %s\n"), *Parameters.PlayerSearchEQS->GetName());
	else
		Result += TEXT("WARNING: Set EQS asset or set Go to player directly to true\n");

	if (Parameters.bInterruptActivePlayerInteraction)
		Result += TEXT("Interrupt active player interaction\n");

	if (!Parameters.SecondaryConversationParticipants.IsEmpty())
		Result += TEXT("Secondary conversation participants");
		
	return Result;
}

#endif

FConstStructView UFlowNode_NpcGoal_TalkToPlayer::GetParametersView()
{
	FConstStructView ParametersConstStructView = FConstStructView(FNpcGoalParameters_TalkToPlayer::StaticStruct(), reinterpret_cast<const uint8*>(&Parameters));
	return ParametersConstStructView;
}
