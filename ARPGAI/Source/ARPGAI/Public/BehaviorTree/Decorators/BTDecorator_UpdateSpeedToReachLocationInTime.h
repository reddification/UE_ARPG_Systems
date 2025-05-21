// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BTService.h"
#include "UObject/Object.h"
#include "BTDecorator_UpdateSpeedToReachLocationInTime.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_UpdateSpeedToReachLocationInTime : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_UpdateSpeedToReachLocationInTime
	{
		FDelegateHandle BlackboardKeyChangedDelegateHandle;
	};
	
public:
	UBTDecorator_UpdateSpeedToReachLocationInTime();
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_UpdateSpeedToReachLocationInTime); };
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetLocationBBKey;

	UPROPERTY(EditAnywhere)
	double MinSpeed = 100.f;

	UPROPERTY(EditAnywhere)
	double MaxSpeed = 900.f;

private:
	EBlackboardNotificationResult OnTargetChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	void UpdateMoveSpeed(const UBlackboardComponent& BlackboardComponent);

	float TimeToGetAtLocation = 1.f;
};
