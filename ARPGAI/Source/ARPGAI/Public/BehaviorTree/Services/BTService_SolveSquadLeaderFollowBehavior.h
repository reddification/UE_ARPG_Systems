// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SolveSquadLeaderFollowBehavior.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_SolveSquadLeaderFollowBehavior : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SolveSquadLeaderFollowBehavior();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector SquadLeaderBBKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutCloseToSquadLeaderBBKey;

	// if the distance between an NPC and its squad leader is more than this value - unset OutCloseToSquadLeader
	UPROPERTY(EditAnywhere)
	float SquadLeaderThresholdDistanceToJoinStatic = 300.f;
};
