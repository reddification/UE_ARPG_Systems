// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_LookForInteractionActor.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_LookForInteractionActor : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_LookForInteractionActor();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutInteractionActorBBKey;

	UPROPERTY(EditAnywhere)
	bool bUseInteractionActorId = true;

	UPROPERTY(EditAnywhere)
	float MinRequiredTimeSeen = 0.75f;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="bUseInteractionActorId"))
	FGameplayTag InteractionActorId;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="bUseInteractionActorId == false"))
	FGameplayTagQuery InteractionActorQuery;
};
