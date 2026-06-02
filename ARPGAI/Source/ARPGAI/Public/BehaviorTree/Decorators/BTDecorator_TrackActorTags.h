// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "Interfaces/NpcActorTagsInterface.h"
#include "BTDecorator_TrackActorTags.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_TrackActorTags : public UBTDecorator
{
	GENERATED_BODY()
	
private:
	struct FBTMemory_TrackActorTags
	{
		FBTMemory_TrackActorTags()
		{
			CurrentActor.Reset();
			DelegateHandle.Reset();
		}
		
		TWeakObjectPtr<AActor> CurrentActor = nullptr;
		FDelegateHandle DelegateHandle;
	};
	
public:
	UBTDecorator_TrackActorTags();
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_TrackActorTags); };
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ActorBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutTagsBBKey;
	
private:
	EBlackboardNotificationResult OnActorChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	void OnActorTagsChanged(AActor* Actor, const FGameplayTagContainer& NewTags, TWeakObjectPtr<UBehaviorTreeComponent> BTComponent);
	void TrackNewActor(TWeakObjectPtr<UBehaviorTreeComponent> BTComponent, UBlackboardComponent* Blackboard, AActor* CurrentActor,
		INpcActorTagsInterface* TagsInterface, FBTMemory_TrackActorTags* BTMemory);
};
