#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsAtLocationSimple.generated.h"

// Unlike built-in decorator for the same purpose, this one doesn't use pathfinding, just direct distance
UCLASS()
class ARPGAI_API UBTDecorator_IsAtLocationSimple : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsAtLocationSimple();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector LocationBBKey;
	
	/** if moving to an actor and this actor has collision volume, use edge of collision volume as target location instead*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 bUseCollisionVolume : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AcceptableDistance = 90.f;
};
