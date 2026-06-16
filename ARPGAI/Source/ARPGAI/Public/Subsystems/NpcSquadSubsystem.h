#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "Interfaces/NpcAliveActor.h"
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

	UFUNCTION(BlueprintCallable)
	void RegisterNpc(const FGameplayTag& NpcIdTag, APawn* Pawn);

	UFUNCTION(BlueprintCallable)
	void UnregisterNpc(const FGameplayTag& NpcIdTag, APawn* Pawn);
	
	UFUNCTION(BlueprintCallable)
	APawn* GetClosestNpc(const FGameplayTag& SquadLeaderTag, APawn* Instigator);
	
	UFUNCTION(BlueprintCallable)
	bool CreateSquad(APawn* SquadLeader, const FGameplayTagContainer& MemberIds, const FGameplayTagQuery& MembersFilter,
	                 const float InRange, int DesiredCount, const FNpcSquadMemberFollowParameters& SquadMemberParameters);
	
	UFUNCTION(BlueprintCallable)
	void JoinOrCreateSquad(APawn* SquadMember, const FGuid& SquadId);

	// Does NOT include self
	UFUNCTION(BlueprintCallable)
	TArray<APawn*> GetAllies(const APawn* RequestorNpc, bool bIgnoreDead) const;
	
	UFUNCTION(BlueprintCallable)
	void LeaveSquad(APawn* Pawn);
	
	UFUNCTION(BlueprintCallable)
	bool IsInSquad(const APawn* Pawn) const;
	
	UFUNCTION(BlueprintCallable)
	void RequestLeaderRole(const APawn* Pawn);
	
	UFUNCTION(BlueprintCallable)
	APawn* GetSquadLeader(const APawn* SquadMember);

	UFUNCTION(BlueprintCallable)
	void DisbandSquad(APawn* SquadLeader);
	
	const FNpcSquadMemberFollowParameters* GetSquadParameters(APawn* ForSquadMember) const;

	void DisbandSquad(const FGuid& SquadId);
	APawn* GetSquadLeader(const FGuid& SquadId);
	
private:
	TMultiMap<FGameplayTag, TWeakObjectPtr<APawn>> NPCs;

	// 1st element is always a squad leader
	// TODO @AK 29.07.2025 consider replacing TWeakObjectPtr to SoftObjectPtr. I assume world partition/level streaming can fuck up these containers
	TMap<FGuid, FSquadData> Squads;
	TMap<TWeakObjectPtr<APawn>, FGuid> SquadsReverseLookup;
	void OnSquadMemberDeathStarted(AActor* Actor, const FNpcDeathEventData& NpcDeathEventData);
};
