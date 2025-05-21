// 


#include "Subsystems/NpcActivitySquadSubsystem.h"

#include "Components/NpcComponent.h"
#include "Components/Controller/NpcActivityComponent.h"

UNpcActivitySquadSubsystem* UNpcActivitySquadSubsystem::Get(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetSubsystem<UNpcActivitySquadSubsystem>();
}

void UNpcActivitySquadSubsystem::RegisterNpc(const FGameplayTag& NpcIdTag, UNpcActivityComponent* NpcActivityComponent)
{
	NPCs.Add(NpcIdTag, NpcActivityComponent);
}

void UNpcActivitySquadSubsystem::UnregisterNpc(const FGameplayTag& SquadLeaderTag, UNpcActivityComponent* NpcActivityComponent)
{
	NPCs.Remove(SquadLeaderTag, NpcActivityComponent);
	if (Squads.Contains(NpcActivityComponent->SquadId))
	{
		// disband squad if it was the leader
		if (Squads[NpcActivityComponent->SquadId][0] == NpcActivityComponent)
		{
			for (int i = 1; i < Squads[NpcActivityComponent->SquadId].Num(); i++)
				Squads[NpcActivityComponent->SquadId][i]->LeaveSquad();

			Squads.Remove(NpcActivityComponent->SquadId);
		}
		else
		{
			Squads[NpcActivityComponent->SquadId].Remove(NpcActivityComponent);
		}
		
		NpcActivityComponent->SquadId.Invalidate();
	}
}

void UNpcActivitySquadSubsystem::CreateSquad(UNpcActivityComponent* SquadLeader, const FGameplayTagContainer& MemberIds, const FGameplayTagQuery& MembersFilter, const float InRange, int DesiredCount,
	const FGameplayTag& SquadMemberAttitudePreset)
{
	TArray<TWeakObjectPtr<UNpcActivityComponent>> PotentialSquadMembers;
	TArray<FGameplayTag> SuitableNpcIds = MemberIds.GetGameplayTagArray();
	const float RangeSq = InRange * InRange;
	int CurrentSquadMembersNum = 0;
	const FVector& SquadLeaderLocation = SquadLeader->GetPawnLocation();
	ensure(!SquadLeader->SquadId.IsValid());

	TArray<TWeakObjectPtr<UNpcActivityComponent>> NewSquad;
	FGuid NewSquadId = FGuid::NewGuid();
	SquadLeader->SquadId = NewSquadId;
	SquadLeader->bSquadLeader = true;
	NewSquad.Add(SquadLeader);
	
	for (const auto& MemberTag : SuitableNpcIds)
	{
		if (CurrentSquadMembersNum >= DesiredCount)
			break;
		
		NPCs.MultiFind(MemberTag, PotentialSquadMembers);
		for (const auto& PotentialSquadMember : PotentialSquadMembers)
		{
			bool bValidSquadMember = ensure(PotentialSquadMember.IsValid())
				&& (PotentialSquadMember->GetPawnLocation() - SquadLeaderLocation).SizeSquared() <= RangeSq
				&& (MembersFilter.IsEmpty() || MembersFilter.Matches(PotentialSquadMember->NpcComponent->GetNpcTags()))
				&& !PotentialSquadMember->SquadId.IsValid();
			
			if (bValidSquadMember)
			{
				CurrentSquadMembersNum++;
				PotentialSquadMember->SquadId = NewSquadId;
				NewSquad.Add(PotentialSquadMember);
				PotentialSquadMember->bSquadLeader = false;
				PotentialSquadMember->NpcComponent->SetAttitudePreset(SquadMemberAttitudePreset);
				
				if (CurrentSquadMembersNum >= DesiredCount)
					break;
			}
		}
	}
	
	Squads.Add(NewSquadId, NewSquad);
}

void UNpcActivitySquadSubsystem::DisbandSquad(UNpcActivityComponent* SquadLeader)
{
	if (!ensure(SquadLeader->SquadId.IsValid()))
		return;

	if (Squads[SquadLeader->SquadId][0] == SquadLeader)
	{
		for (int i = 1; i < Squads[SquadLeader->SquadId].Num(); i++)
			Squads[SquadLeader->SquadId][i]->LeaveSquad();

		Squads.Remove(SquadLeader->SquadId);
	}

	SquadLeader->SquadId.Invalidate();
	SquadLeader->bSquadLeader = false;
}

TArray<APawn*> UNpcActivitySquadSubsystem::GetAllies(const APawn* RequestorNpc, bool bIgnoreSquadLeader) const
{
	TArray<APawn*> Result;
	// TODO refactor
	auto NpcActivityComponent = RequestorNpc->GetController()->FindComponentByClass<UNpcActivityComponent>();
	if (!NpcActivityComponent->SquadId.IsValid())
		return Result;

	for (const auto& SquadMember : Squads[NpcActivityComponent->SquadId])
	{
		if (SquadMember->bSquadLeader && bIgnoreSquadLeader)
			continue;
		
		if (ensure(SquadMember.IsValid()) && SquadMember->GetNpcPawn() != RequestorNpc)
			Result.Add(SquadMember->GetNpcPawn());
	}

	return Result;
}

void UNpcActivitySquadSubsystem::SetGoalForSquadMembers(const FGuid& SquadId, const FNpcGoalChain& SubordinateGoalChain)
{
	const auto* SquadPtr = Squads.Find(SquadId);
	if (ensure(SquadPtr))
	{
		for (int i = 1; i < SquadPtr->Num(); i++)
		{
			(*SquadPtr)[i]->SetSubordinateNpcGoal(SubordinateGoalChain);
		}
	}
}

void UNpcActivitySquadSubsystem::ResetGoalForSquadMembers(const FGuid& SquadId)
{
	const auto* SquadPtr = Squads.Find(SquadId);
	if (ensure(SquadPtr))
	{
		for (int i = 1; i < SquadPtr->Num(); i++)
		{
			(*SquadPtr)[i]->ResetSubordinateGoal();
		}
	}
}

UNpcActivityComponent* UNpcActivitySquadSubsystem::GetSquadLeader(const FGuid& SquadId)
{
	if (!SquadId.IsValid())
		return nullptr;

	auto SquadPtr = Squads.Find(SquadId);
	if (!ensure(SquadPtr))
		return nullptr;

	return (*SquadPtr)[0].Get();
}

UNpcActivityComponent* UNpcActivitySquadSubsystem::GetClosestNpc(const FGameplayTag& SquadLeaderTag, APawn* Instigator)
{
	TArray<TWeakObjectPtr<UNpcActivityComponent>> PotentialSquadLeaders;
	NPCs.MultiFind(SquadLeaderTag, PotentialSquadLeaders);

	if (PotentialSquadLeaders.Num() == 0)
		return nullptr;

	const FVector& InstigatorLocation = Instigator->GetActorLocation();
	float ClosestDistSq = FLT_MAX;
	UNpcActivityComponent* ClosestSquadLeader = nullptr;
	for (const auto& PotentialSquadLeader : PotentialSquadLeaders)
	{
		if (!PotentialSquadLeader.IsValid())
			continue;
		
		const float DistSq = (PotentialSquadLeader->GetPawnLocation() - InstigatorLocation).SizeSquared();
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestSquadLeader = PotentialSquadLeader.Get();
		}
	}

	return ClosestSquadLeader;
}
