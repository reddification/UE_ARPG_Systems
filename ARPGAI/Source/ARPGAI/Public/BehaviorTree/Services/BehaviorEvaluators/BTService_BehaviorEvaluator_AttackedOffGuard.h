// 

#pragma once

#include "CoreMinimal.h"
#include "BTService_BehaviorEvaluator_Base.h"
#include "Perception/AIPerceptionTypes.h"
#include "BTService_BehaviorEvaluator_AttackedOffGuard.generated.h"

/**
 * 
 */
UCLASS(meta=(Category="Behavior evaluators"))
class ARPGAI_API UBTService_BehaviorEvaluator_AttackedOffGuard : public UBTService_BehaviorEvaluator_Base
{
	GENERATED_BODY()

	struct FBTMemory_BehaviorEvaluator_AttackedOffGuard : public FBTMemory_BehaviorEvaluator_Base
	{
		FVector AttackedFromLocation = FVector::ZeroVector;
		TWeakObjectPtr<AActor> Attacker;
	};
	
public:
	UBTService_BehaviorEvaluator_AttackedOffGuard();
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_BehaviorEvaluator_AttackedOffGuard); };
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ReceivedHitFromLocationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AttackerActorBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PerceiveDamageCauserDistanceThreshold = 400.f;

public: //IBlackboardObserver
	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual void FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	
private:
	bool CanObserve(const APawn* NpcPawn, const AActor* AttackerActor) const;
	void OnPerceptionUpdated(AActor* Actor, const FAIStimulus& FaiStimulus, UBehaviorTreeComponent* BehaviorTreeComponent);
};
