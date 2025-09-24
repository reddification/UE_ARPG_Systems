// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/NpcComponent.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "UObject/Object.h"
#include "NpcSquadSubsystem.generated.h"

class UNpcActivityComponent;
struct FGameplayTag;

struct FSquadData
{
	TArray<TWeakObjectPtr<APawn>> SquadMembers;
	FNpcSquadMemberFollowParameters SquadParameters;

	TWeakObjectPtr<APawn>& operator[](const int32 index)
	{
		return SquadMembers[index];
	}

	const TWeakObjectPtr<APawn>& operator[](const int32 index) const
	{
		return SquadMembers[index];
	}
};

UCLASS()
class ARPGAI_API UNpcSquadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UNpcSquadSubsystem* Get(const UObject* WorldContextObject);
	
	void RegisterNpc(const FGameplayTag& NpcIdTag, APawn* Pawn);
	void UnregisterNpc(const FGameplayTag& NpcIdTag, APawn* Pawn);
	APawn* GetClosestNpc(const FGameplayTag& SquadLeaderTag, APawn* Instigator);
	bool CreateSquad(APawn* SquadLeader, const FGameplayTagContainer& MemberIds, const FGameplayTagQuery& MembersFilter,
	                 const float InRange, int DesiredCount, const FNpcSquadMemberFollowParameters& SquadMemberParameters);
	void JoinSquad(APawn* SquadMember, const FGuid& SquadId);
	void DisbandSquad(const FGuid& SquadId);
	void DisbandSquad(APawn* SquadLeader);

	TArray<APawn*> GetAllies(const APawn* RequestorNpc, bool bIgnoreSquadLeader, bool bIgnoreDead) const;
	const FNpcSquadMemberFollowParameters* GetSquadParameters(APawn* ForSquadMember) const;
	APawn* GetSquadLeader(const FGuid& SquadId);
	APawn* GetSquadLeader(const APawn* SquadMember);
	void LeaveSquad(APawn* Pawn);
	bool IsInSquad(const APawn* Pawn) const;
	void RequestLeaderRole(const APawn* Pawn);

private:
	TMultiMap<FGameplayTag, TWeakObjectPtr<APawn>> NPCs;

	// 1st element is always a squad leader
	// TODO @AK 29.07.2025 consider replacing TWeakObjectPtr to SoftObjectPtr. I assume world partition/level streaming can fuck up these containers
	TMap<FGuid, FSquadData> Squads;
	TMap<TWeakObjectPtr<APawn>, FGuid> SquadsReverseLookup;
};
