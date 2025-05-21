

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/Services/BTService_DefaultFocus.h"
#include "BTService_SetFocus.generated.h"

struct FBTMemory_AIFocus : public FBTAuxiliaryMemory
{
	uint8 ActualFocusPriority = 0;
	AActor* FocusActorSet;
	FVector FocusLocationSet;
	bool bActorSet;

	void Reset()
	{
		FocusActorSet = nullptr;
		FocusLocationSet = FAISystem::InvalidLocation;
		bActorSet = false;
	}
};


UCLASS(hidecategories=(Service))
class ARPGAI_API UBTService_SetFocus : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetFocus();
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_AIFocus); }
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	EBlackboardNotificationResult OnBlackboardKeyValueChangeWithPriority(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);

	/** blackboard key selector */
	UPROPERTY(EditAnywhere, Category=Blackboard)
	struct FBlackboardKeySelector BlackboardKey;
	
#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR
};
