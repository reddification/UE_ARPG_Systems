// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BTService_RunEQS_NpcGoal.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_RunEQS_NpcGoal : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
private:
	struct FBTEQSServiceMemory
	{
		/** Query request ID */
		int32 RequestID;
	};
	
public:
	UBTService_RunEQS_NpcGoal();
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTEQSServiceMemory); }

	virtual FString GetStaticDescription() const override;

	UPROPERTY(EditAnywhere)
	FGameplayTag EqsId;
	
private:
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);
	FQueryFinishedSignature QueryFinishedDelegate;
};
