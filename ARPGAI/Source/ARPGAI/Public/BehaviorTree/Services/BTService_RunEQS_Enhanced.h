

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_RunEQS.h"
#include "BTService_RunEQS_Enhanced.generated.h"

UCLASS()
class ARPGAI_API UBTService_RunEQS_Enhanced : public UBTService_RunEQS
{
	GENERATED_BODY()

private:
	struct FBTMemory_RunEQSWithThreshold : public FBTEQSServiceMemory
	{
		FVector PreviousLocation = FVector::ZeroVector;
		FDelegateHandle GateObserverDelegateHandle;
	};
	
public:
	UBTService_RunEQS_Enhanced(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_RunEQSWithThreshold); };

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	bool TryAddCustomParamToEQS(const FName& CustomParamName);
#endif
	
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ScoreUpdateThreshold = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float ScoreUpdateThresholdCountRatio = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ThresholdDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector GateBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bInversedGate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSet<FName> CustomEqsParamsNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer CustomEqsParamsTagNames;
	
	virtual void OnQueryFinished2(TSharedPtr<FEnvQueryResult> Result);
	
private:
	EBlackboardNotificationResult OnGateUpdated(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);

};
