// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "NpcSquadMemberComponent.generated.h"


struct FGameplayTag;
struct FNpcDTR;
class AAIController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcSquadMemberComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void Initialize(const FDataTableRowHandle& NpcDTRH);

	bool FollowLeader();
	void StopFollowing();

	void SetFormationWalkingIndex(int InGroupWalkingIndex);
	void SetFormationWalkingEnabled(bool bEnabled);
	void ResetFormationWalking();
	
	void LeaveSquad();
	
	void OnEnteredSquad(bool bSquadLeader);
	void OnSquadLeft();
	void AddBehaviorCooldownToAllies(const FGameplayTag& CooldownTag, float Time);

private:
	TWeakObjectPtr<AAIController> OwnerController;
	int GroupWalkingIndex = 0;
	
	UPROPERTY()
	TObjectPtr<UNpcBlackboardDataAsset> BlackboardKeys;
};
