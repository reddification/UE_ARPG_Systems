// 


#include "Subsystems/NpcSquadSubsystem.h"

#include "Components/NpcAttitudesComponent.h"
#include "Components/Controller/NpcSquadMemberComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcAliveCreature.h"

UNpcSquadSubsystem* UNpcSquadSubsystem::Get(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetSubsystem<UNpcSquadSubsystem>();
}

void UNpcSquadSubsystem::RegisterNpc(const FGameplayTag& NpcIdTag, APawn* Pawn)
{
	NPCs.Add(NpcIdTag, Pawn);
}

void UNpcSquadSubsystem::UnregisterNpc(const FGameplayTag& SquadLeaderTag, APawn* Pawn)
{
	NPCs.Remove(SquadLeaderTag, Pawn);
	if (!SquadsReverseLookup.Contains(Pawn))
		return;

	const auto& SquadId  = SquadsReverseLookup[Pawn];
	if (!Squads.Contains(SquadId))
	{
		UE_VLOG(Pawn, LogARPGAI, Warning, TEXT("UNpcSquadSubsystem::UnregisterNpc: pawn tried to unregister from a squad that does not exist"));
		return;
	}
		// disband squad if it was the leader
	if (Squads[SquadId].SquadMembers[0] == Pawn)
	{
		DisbandSquad(SquadId);
	}
	else
	{
		auto Controller = Pawn->GetController();
		if (IsValid(Controller))
		{
			auto SquadMemberComponent = Controller->FindComponentByClass<UNpcSquadMemberComponent>();
			if (IsValid(SquadMemberComponent))
				SquadMemberComponent->OnSquadLeft();
		}
		
		Squads[SquadId].SquadMembers.Remove(Pawn);
		SquadsReverseLookup.Remove(Pawn);
	}
}

bool UNpcSquadSubsystem::CreateSquad(APawn* SquadLeader, const FGameplayTagContainer& MemberIds,
                                     const FGameplayTagQuery& MembersFilter, const float InRange, int DesiredCount,
                                     const FNpcSquadMemberFollowParameters& SquadMemberParameters)
{
	if (!ensure(!SquadsReverseLookup.Contains(SquadLeader)))
		return false;
	
	TArray<TWeakObjectPtr<APawn>> PotentialSquadMembers;
	TArray<FGameplayTag> SuitableNpcIds = MemberIds.GetGameplayTagArray();
	const float RangeSq = InRange * InRange;
	int CurrentSquadMembersNum = 0;
	const FVector& SquadLeaderLocation = SquadLeader->GetActorLocation();

	TArray<TWeakObjectPtr<APawn>> NewSquad;
	TArray<TPair<APawn*, UNpcSquadMemberComponent*>> NewSquadMembers;
	
	for (const auto& MemberTag : SuitableNpcIds)
	{
		if (CurrentSquadMembersNum >= DesiredCount)
			break;
		
		NPCs.MultiFind(MemberTag, PotentialSquadMembers);
		for (const auto& PotentialSquadMember : PotentialSquadMembers)
		{
			bool bValidSquadMember = PotentialSquadMember.IsValid()
				&& !SquadsReverseLookup.Contains(PotentialSquadMember.Get())
				&& (PotentialSquadMember->GetActorLocation() - SquadLeaderLocation).SizeSquared() <= RangeSq;
			
			if (!bValidSquadMember)
				continue;

			auto PotentialMemberNpcComponent = PotentialSquadMember->GetController()->FindComponentByClass<UNpcSquadMemberComponent>();
			if (!PotentialMemberNpcComponent)
				continue;

			if (!MembersFilter.IsEmpty())
			{
				auto Npc = Cast<INpc>(PotentialSquadMember.Get());
				bValidSquadMember = MembersFilter.Matches(Npc->GetNpcOwnerTags());
			}
			
			if (bValidSquadMember)
			{
				NewSquadMembers.Add( { PotentialSquadMember.Get(), PotentialMemberNpcComponent } );
				CurrentSquadMembersNum++;
				if (CurrentSquadMembersNum >= DesiredCount)
					break;
			}
		}
	}

	if (NewSquadMembers.IsEmpty())
		return false;

	FGuid NewSquadId = FGuid::NewGuid();
	auto SquadLeaderNpcComponent = SquadLeader->GetController()->FindComponentByClass<UNpcSquadMemberComponent>();
	SquadLeaderNpcComponent->OnEnteredSquad(true);
	NewSquad.Add(SquadLeader);
	SquadsReverseLookup.Add(SquadLeader, NewSquadId);
	
	for (const auto& SquadMember : NewSquadMembers)
	{
		SquadMember.Value->OnEnteredSquad(false);
		NewSquad.Add(SquadMember.Key);
		SquadsReverseLookup.Add(SquadMember.Key, NewSquadId);
	}

	FSquadData Data;
	Data.SquadMembers = MoveTemp(NewSquad);
	Data.SquadParameters = SquadMemberParameters;
	Squads.Add(NewSquadId, Data);
	return true;
}

void UNpcSquadSubsystem::JoinSquad(APawn* SquadMember, const FGuid& SquadId)
{
	if (!Squads.Contains(SquadId))
		Squads.Add(SquadId, FSquadData());

	Squads[SquadId].SquadMembers.Add(SquadMember);
	SquadsReverseLookup.Add(SquadMember, SquadId);
}

void UNpcSquadSubsystem::DisbandSquad(APawn* SquadLeader)
{
	if (!SquadsReverseLookup.Contains(SquadLeader))
		return;
	
	const auto& SquadId = SquadsReverseLookup[SquadLeader];
	if (Squads[SquadId][0] == SquadLeader)
		DisbandSquad(SquadId);
}

void UNpcSquadSubsystem::DisbandSquad(const FGuid& SquadId)
{
	if (!Squads.Contains(SquadId))
		return;
	
	for (int i = 0; i < Squads[SquadId].SquadMembers.Num(); i++)
	{
		if (!Squads[SquadId][i].IsValid())
			continue;

		auto AIController = Squads[SquadId][i]->GetController();
		if (!IsValid(AIController))
			continue;
		
		AIController->FindComponentByClass<UNpcSquadMemberComponent>()->OnSquadLeft();
		SquadsReverseLookup.Remove(Squads[SquadId][i]);
	}
	
	Squads.Remove(SquadId);
}

TArray<APawn*> UNpcSquadSubsystem::GetAllies(const APawn* RequestorNpc, bool bIgnoreSquadLeader, bool bIgnoreDead) const
{
	TArray<APawn*> Result;
	if (!SquadsReverseLookup.Contains(RequestorNpc))
		return Result;

	const FGuid& SquadId = SquadsReverseLookup[RequestorNpc];
	bool bSquadLeader = Squads[SquadId][0] == RequestorNpc;
	for (const auto& SquadMember : Squads[SquadId].SquadMembers)
	{
		if (bSquadLeader && bIgnoreSquadLeader)
			continue;

		if (bIgnoreDead)
			if (auto AliveInterface = Cast<INpcAliveCreature>(SquadMember.Get()))
				if (!AliveInterface->IsNpcActorAlive())
					continue;
		
		if (ensure(SquadMember.IsValid()) && SquadMember != RequestorNpc)
			Result.Add(SquadMember.Get());
	}

	return Result;
}

const FNpcSquadMemberFollowParameters* UNpcSquadSubsystem::GetSquadParameters(APawn* ForSquadMember) const
{
	const auto* SquadIdPtr = SquadsReverseLookup.Find(ForSquadMember);
	if (SquadIdPtr == nullptr)
		return nullptr;

	return &Squads[*SquadIdPtr].SquadParameters;
}

APawn* UNpcSquadSubsystem::GetSquadLeader(const APawn* SquadMember)
{
	auto SquadIdPtr = SquadsReverseLookup.Find(SquadMember);
	return SquadIdPtr ? GetSquadLeader(*SquadIdPtr) : nullptr;
}

APawn* UNpcSquadSubsystem::GetSquadLeader(const FGuid& SquadId)
{
	auto SquadPtr = Squads.Find(SquadId);
	if (!ensure(SquadPtr))
		return nullptr;

	return SquadPtr->SquadMembers[0].Get();
}

void UNpcSquadSubsystem::LeaveSquad(APawn* Pawn)
{
	if (!SquadsReverseLookup.Contains(Pawn))
		return;

	Squads[SquadsReverseLookup[Pawn]].SquadMembers.Remove(Pawn);
	SquadsReverseLookup.Remove(Pawn);
}

bool UNpcSquadSubsystem::IsInSquad(const APawn* Pawn) const
{
	return SquadsReverseLookup.Contains(Pawn);
}

void UNpcSquadSubsystem::RequestLeaderRole(const APawn* Pawn)
{
	auto PawnSquadId = SquadsReverseLookup.Find(Pawn);
	if (PawnSquadId != nullptr)
	{
		int PawnIndex = Squads[*PawnSquadId].SquadMembers.IndexOfByKey(Pawn);
		if (PawnIndex != 0)
			Squads[*PawnSquadId].SquadMembers.Swap(0, PawnIndex);
	}
}

APawn* UNpcSquadSubsystem::GetClosestNpc(const FGameplayTag& SquadLeaderTag, APawn* Instigator)
{
	TArray<TWeakObjectPtr<APawn>> PotentialSquadLeaders;
	NPCs.MultiFind(SquadLeaderTag, PotentialSquadLeaders);

	if (PotentialSquadLeaders.Num() == 0)
		return nullptr;

	const FVector& InstigatorLocation = Instigator->GetActorLocation();
	float ClosestDistSq = FLT_MAX;
	APawn* ClosestSquadLeader = nullptr;
	for (const auto& PotentialSquadLeader : PotentialSquadLeaders)
	{
		if (!PotentialSquadLeader.IsValid())
			continue;
		
		const float DistSq = (PotentialSquadLeader->GetActorLocation() - InstigatorLocation).SizeSquared();
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestSquadLeader = PotentialSquadLeader.Get();
		}
	}

	return ClosestSquadLeader;
}
