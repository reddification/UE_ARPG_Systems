#include "Components/Controller/NpcSquadMemberComponent.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Subsystems/NpcSquadSubsystem.h"

void UNpcSquadMemberComponent::Initialize(const FDataTableRowHandle& InNpcDTRH)
{
	NpcDTRH = InNpcDTRH;
	OwnerController = Cast<AAIController>(GetOwner());
	ensure(OwnerController.IsValid());
}

bool UNpcSquadMemberComponent::FollowLeader()
{
	auto Pawn = OwnerController->GetPawn();
	auto SquadSubsystem = UNpcSquadSubsystem::Get(Pawn);
	if (!SquadSubsystem->IsInSquad(Pawn))
		return false;

	auto SquadParameters = SquadSubsystem->GetSquadParameters(Pawn);
	if (!ensure(SquadParameters))
		return false;

	auto BlackboardComponent = OwnerController->GetBlackboardComponent();
	auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset;
	switch (SquadParameters->NpcSquadFollowType)
	{
		case ENpcSquadFollowType::Around:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, 0.f);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, 0.f);
			break;
		case ENpcSquadFollowType::InFront:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, SquadParameters->DotProductScore);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, 0.85f);
			break;
		case ENpcSquadFollowType::NextTo:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, SquadParameters->DotProductScore);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, 0.0f);
			break;
		case ENpcSquadFollowType::Behind:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, SquadParameters->DotProductScore);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, -0.85f);
			break;
		case ENpcSquadFollowType::NotInFrontOf:
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductFactorBBKey.SelectedKeyName, -SquadParameters->DotProductScore);
			BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderDotProductDesiredDirectionBBKey.SelectedKeyName, 0.85);
			break;
		default:
			ensure(false);
			break;
	}

	auto SquadLeader = SquadSubsystem->GetSquadLeader(Pawn);
	BlackboardComponent->SetValueAsFloat(BlackboardKeys->FollowLeaderCircleRadiusBBKey.SelectedKeyName, SquadParameters->FollowRadius);
	BlackboardComponent->SetValueAsObject(BlackboardKeys->FollowTargetBBKey.SelectedKeyName, SquadLeader);
	return true;
}

void UNpcSquadMemberComponent::StopFollowing()
{
	auto BlackboardComponent = OwnerController->GetBlackboardComponent();
	auto BlackboardKeys = GetNpcDTR()->NpcBlackboardDataAsset;
	BlackboardComponent->ClearValue(BlackboardKeys->FollowTargetBBKey.SelectedKeyName);
}

// @AK 14.07.2025: Copied from another project. Not used yet
// it should be called when NPCs start to walk in patrols
// the index should be controlled by a single entity, a subsystem perhaps
void UNpcSquadMemberComponent::SetFormationWalkingIndex(int InGroupWalkingIndex)
{
	if (!ensure(InGroupWalkingIndex >= 1 && InGroupWalkingIndex < 31))
		return;

	GroupWalkingIndex = InGroupWalkingIndex;
	if (auto CrowdFollowingComponent = Cast<UCrowdFollowingComponent>(OwnerController->GetPathFollowingComponent()))
		CrowdFollowingComponent->SetAvoidanceGroup(1 << GroupWalkingIndex);
}

void UNpcSquadMemberComponent::SetFormationWalkingEnabled(bool bEnabled)
{
	if (auto CrowdFollowingComponent = Cast<UCrowdFollowingComponent>(OwnerController->GetPathFollowingComponent()))
	{
		if (bEnabled)
		{
			ensure(CrowdFollowingComponent->GetAvoidanceGroup() > 1);
			CrowdFollowingComponent->SetGroupsToIgnore(CrowdFollowingComponent->GetAvoidanceGroup());
		}
		else
		{
			CrowdFollowingComponent->SetGroupsToIgnore(0);
		}
	}
}

void UNpcSquadMemberComponent::ResetFormationWalking()
{
	GroupWalkingIndex = -1;
	if (auto CrowdFollowingComponent = Cast<UCrowdFollowingComponent>(OwnerController->GetPathFollowingComponent()))
	{
		CrowdFollowingComponent->SetAvoidanceGroup(1);
		CrowdFollowingComponent->SetGroupsToAvoid(MAX_uint32);
	}
}

void UNpcSquadMemberComponent::LeaveSquad()
{
	auto SquadSubsystem = UNpcSquadSubsystem::Get(this);
	SquadSubsystem->LeaveSquad(OwnerController->GetPawn());
}

void UNpcSquadMemberComponent::OnEnteredSquad(bool bSquadLeader)
{
	
}

void UNpcSquadMemberComponent::OnSquadLeft()
{
}

void UNpcSquadMemberComponent::AddBehaviorCooldownToAllies(const FGameplayTag& CooldownTag, float Time)
{
	auto SquadSubsystem = UNpcSquadSubsystem::Get(this);
	if (!SquadSubsystem)
		return;

	auto Allies = SquadSubsystem->GetAllies(OwnerController->GetPawn(), false, true);
	for (const auto& Ally : Allies)
	{
		auto AllyController = Ally->GetController<AAIController>();
		auto AllyBTComponent = Cast<UBehaviorTreeComponent>(AllyController->GetBrainComponent());
		AllyBTComponent->AddCooldownTagDuration(CooldownTag, Time, false);
	}
}

const FNpcDTR* UNpcSquadMemberComponent::GetNpcDTR() const
{
	if (ensure(NpcDTRH.DataTable && NpcDTRH.RowName.IsNone()))
		return NpcDTRH.GetRow<FNpcDTR>("");

	return nullptr;
}
