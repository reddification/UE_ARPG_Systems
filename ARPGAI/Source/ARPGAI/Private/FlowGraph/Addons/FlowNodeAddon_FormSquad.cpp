#include "FlowGraph/Addons/FlowNodeAddon_FormSquad.h"

#include "AIController.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/Controller/NpcPerceptionReactionComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Components/Controller/NpcSquadMemberComponent.h"
#include "FlowGraph/Nodes/FlowNode_NpcGoal.h"
#include "Subsystems/NpcSquadSubsystem.h"

void UFlowNodeAddon_FormSquad::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	auto AIController = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto NpcSquadSubsystem = UNpcSquadSubsystem::Get(AIController);
	auto SquadLeaderPawn = AIController->GetPawn();
	bSquadCreated = NpcSquadSubsystem->CreateSquad(SquadLeaderPawn, SuitableSquadMembersIds, SuitableSquadMembersFilter, FollowersInRange,
	                                               DesiredFollowers, SquadMemberParameters);
	if (bSquadCreated)
	{
		auto SquadMembers = NpcSquadSubsystem->GetAllies(SquadLeaderPawn, true, true);
		for (auto SquadMember : SquadMembers)
		{
			if (SquadMemberAttitudePreset.IsValid())
			{
				auto SquadMemberAttitudesComponent = SquadMember->FindComponentByClass<UNpcAttitudesComponent>();
				SquadMemberAttitudesComponent->SetTemporaryAttitudePreset(SquadMemberAttitudePreset);
			}

			if (!PerceptionReactionEvaluators.IsEmpty())
			{
				auto SquadMemberPerceptionEvaluation = SquadMember->GetController()->FindComponentByClass<UNpcPerceptionReactionComponent>();
				SquadMemberPerceptionEvaluation->AddReactionBehaviorEvaluators(PerceptionReactionEvaluators);
			}
			
			auto NpcSquadMemberComponent = SquadMember->GetController()->FindComponentByClass<UNpcSquadMemberComponent>();
			NpcSquadMemberComponent->FollowLeader();
		}
	}
}

void UFlowNodeAddon_FormSquad::FinishState()
{
	if (!bSquadCreated)
		return;
	
	if (auto AIController = Cast<AAIController>(TryGetRootFlowActorOwner()))
	{
		if (auto NpcSquadSubsystem = UNpcSquadSubsystem::Get(AIController))
		{
			auto SquadLeaderPawn = AIController->GetPawn();
			auto SquadMembers = NpcSquadSubsystem->GetAllies(SquadLeaderPawn, true, true);
			for (auto SquadMember : SquadMembers)
			{
				if (SquadMemberAttitudePreset.IsValid())
				{
					auto SquadMemberAttitudesComponent = SquadMember->FindComponentByClass<UNpcAttitudesComponent>();
					SquadMemberAttitudesComponent->ResetTemporaryAttitudePreset();
				}

				if (!PerceptionReactionEvaluators.IsEmpty())
				{
					auto SquadMemberPerceptionEvaluation = SquadMember->GetController()->FindComponentByClass<UNpcPerceptionReactionComponent>();
					SquadMemberPerceptionEvaluation->RemoveReactionBehaviorEvaluators(PerceptionReactionEvaluators);
				}

				auto NpcSquadMemberComponent = SquadMember->GetController()->FindComponentByClass<UNpcSquadMemberComponent>();
				NpcSquadMemberComponent->StopFollowing();
			}
			
			NpcSquadSubsystem->DisbandSquad(AIController->GetPawn());
		}
	}
	
	Super::FinishState();
}

EFlowAddOnAcceptResult UFlowNodeAddon_FormSquad::AcceptFlowNodeAddOnParent_Implementation(
	const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return ParentTemplate->IsA<UFlowNode_NpcGoal>() ? EFlowAddOnAcceptResult::TentativeAccept : EFlowAddOnAcceptResult::Reject;
}
