// 


#include "FlowGraph/Nodes/FlowNode_NpcGoal_Conversate.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Struct.h"
#include "Components/NpcComponent.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "Subsystems/NpcRegistrationSubsystem.h"

ENpcGoalStartResult UFlowNode_NpcGoal_Conversate::Start()
{
	auto Result = Super::Start();
	if (Result != ENpcGoalStartResult::InProgress)
		return Result;
	
	return ENpcGoalStartResult::InProgress;
}

ENpcGoalStartResult UFlowNode_NpcGoal_Conversate::Restore(bool bInitialStart)
{
	Super::Restore(bInitialStart);
	bool bSetConversationBlackboardContext = SetConversationBlackboardContext();
	return bSetConversationBlackboardContext ? ENpcGoalStartResult::InProgress : ENpcGoalStartResult::Failed;
}

bool UFlowNode_NpcGoal_Conversate::SetConversationBlackboardContext() const
{
	if (Parameters.bUseEQS)
	{
		BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, Parameters.ConversationPartnersEQS);
		return true;
	}
	else if (Parameters.ConversationPartnerId.IsValid())
	{
		auto NpcRegistrationSubsystem = UNpcRegistrationSubsystem::Get(NpcPawn.Get());
		auto ConversationPartner = NpcRegistrationSubsystem->GetClosestNpc(Parameters.ConversationPartnerId, NpcPawn->GetActorLocation(),
			&Parameters.ConversationPartnerTagsFilter);
		if (ConversationPartner)
		{
			BlackboardComponent->SetValueAsObject(BlackboardKeys->ConversationPartnerBBKey.SelectedKeyName, ConversationPartner->GetOwner());
			return true;
		}
	}
	
	return false;
}

FConstStructView UFlowNode_NpcGoal_Conversate::GetParametersView()
{
	FConstStructView ParametersConstStructView = FConstStructView(FNpcGoalParameters_Conversate::StaticStruct(), reinterpret_cast<const uint8*>(&Parameters));
	return ParametersConstStructView;
}

#if WITH_EDITOR

EDataValidationResult UFlowNode_NpcGoal_Conversate::ValidateNode()
{
	return Super::ValidateNode();
}

FString UFlowNode_NpcGoal_Conversate::GetGoalDescription() const
{
	return Super::GetGoalDescription();
}

#endif