// 

#pragma once

#include "CoreMinimal.h"
#include "BTService_BehaviorEvaluator_Base.h"
#include "BTService_BehaviorEvaluator_ProximityAwareness.generated.h"

/**
 * 
 */
UCLASS(meta=(Category="Behavior evaluators"))
class ARPGAI_API UBTService_BehaviorEvaluator_ProximityAwareness : public UBTService_BehaviorEvaluator_Base
{
	GENERATED_BODY()

private:
	struct FBTMemory_BE_WakeUp : public FBTMemory_BehaviorEvaluator_Base
	{
		FVector AwarenessLocation = FAISystem::InvalidLocation;
	};
	
public:
	UBTService_BehaviorEvaluator_ProximityAwareness();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_BE_WakeUp); };

	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual void FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WakeUpToNoiseChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AwarenessLocationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer WakeUpToNoises;
};
