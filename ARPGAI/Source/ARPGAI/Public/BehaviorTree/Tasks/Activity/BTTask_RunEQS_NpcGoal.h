#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BTTask_RunEQS_NpcGoal.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_RunEQS_NpcGoal : public UBTTaskNode
{
	GENERATED_BODY()
	
private:
	struct FBTMemory_RunActivityEQS
	{
		int32 RequestID;
	};
	
public:
	UBTTask_RunEQS_NpcGoal();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_RunActivityEQS); };
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag EqsId;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ResultBBKey;

#if WITH_EDITOR
	/** prepare query params */
	virtual FName GetNodeIconName() const override;
#endif

	
private:
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);
	FQueryFinishedSignature QueryFinishedDelegate;
};
