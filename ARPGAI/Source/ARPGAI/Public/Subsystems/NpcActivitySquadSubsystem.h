// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/NpcComponent.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "UObject/Object.h"
#include "NpcActivitySquadSubsystem.generated.h"

class UNpcActivityComponent;
struct FGameplayTag;

struct FNpcSquadMemberData
{
};

UCLASS()
class ARPGAI_API UNpcActivitySquadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UNpcActivitySquadSubsystem* Get(const UObject* WorldContextObject);
	
	void RegisterNpc(const FGameplayTag& NpcIdTag, UNpcActivityComponent* NpcActivityComponent);
	void UnregisterNpc(const FGameplayTag& NpcIdTag, UNpcActivityComponent* NpcActivityComponent);
	UNpcActivityComponent* GetClosestNpc(const FGameplayTag& SquadLeaderTag, APawn* Instigator);
	void CreateSquad(UNpcActivityComponent* SquadLeader, const FGameplayTagContainer& MemberIds, const FGameplayTagQuery& MembersFilter, const float InRange, int DesiredCount,
		const FGameplayTag& SquadMemberAttitudePreset);
	void DisbandSquad(UNpcActivityComponent* SquadLeader);
	TArray<APawn*> GetAllies(const APawn* RequestorNpc, bool bIgnoreSquadLeader) const;
	void SetGoalForSquadMembers(const FGuid& SquadId, const FNpcGoalChain& SubordinateGoalChain);
	void ResetGoalForSquadMembers(const FGuid& SquadId);
	UNpcActivityComponent* GetSquadLeader(const FGuid& SquadId);

private:
	TMultiMap<FGameplayTag, TWeakObjectPtr<UNpcActivityComponent>> NPCs;

	// 1st element is always a squad leader
	TMap<FGuid, TArray<TWeakObjectPtr<UNpcActivityComponent>>> Squads;
};
