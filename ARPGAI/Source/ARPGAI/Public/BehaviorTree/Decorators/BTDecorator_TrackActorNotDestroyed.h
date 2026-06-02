// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_TrackActorNotDestroyed.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_TrackActorNotDestroyed : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_TrackActorNotDestroyed
	{
		FBTMemory_TrackActorNotDestroyed()
		{
			CurrentActor.Reset();
			DestroyDelegateHandle.Reset();
		}
		
		TWeakObjectPtr<AActor> CurrentActor = nullptr;
		FDelegateHandle DestroyDelegateHandle;
	};
	
public:
	UBTDecorator_TrackActorNotDestroyed();
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_TrackActorNotDestroyed); };
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ActorBBKey;
	
private:
	void OnActorDestroyed(AActor* Actor, AActor* Instigator, TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerBT);
	void ResetCurrentActor(FBTMemory_TrackActorNotDestroyed* BTMemory);
	EBlackboardNotificationResult OnActorChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	
};
