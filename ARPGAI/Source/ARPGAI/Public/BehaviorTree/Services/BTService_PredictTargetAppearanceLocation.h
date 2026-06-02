// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_PredictTargetAppearanceLocation.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_PredictTargetAppearanceLocation : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_PredictTargetAppearanceLocation();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector LastKnownTargetLocationBBKey;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer AttentionSoundTags;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutExpectedLocationBBKey;
	
	UPROPERTY(EditAnywhere)
	bool bPathFromTargetToNpc = true;

	UPROPERTY(EditAnywhere)
	bool bUseHierarchicalPathfinding = false;
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> SweepTraceChannel;
};
