#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "UObject/Object.h"
#include "BTTask_FindAndClaimActivitySmartObject.generated.h"

struct FSmartObjectClaimHandle;
class USmartObjectSubsystem;
UCLASS(Category="Smart Objects")
class ARPGAI_API UBTTask_FindAndClaimActivitySmartObject : public UBTTaskNode
{
	GENERATED_BODY()

private:
	struct FBTMemory_FindAndClaimSmartObject
	{
		int32 EQSRequestID;
	};
	
public:
	UBTTask_FindAndClaimActivitySmartObject();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_FindAndClaimSmartObject); }
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector InteractionActorBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutSmartObjectSlotLocationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutSmartObjectSlotRotationBBKey;

	UPROPERTY(EditAnywhere, Category = SmartObjects)
	FBlackboardKeySelector OutSmartObjectClaimHandleBBKey;	
	
	UPROPERTY(EditAnywhere, Category = SmartObjects)
	FEQSParametrizedQueryExecutionRequest EQSRequest;

private:
	FQueryFinishedSignature EQSQueryFinishedDelegate;

	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);
	void ClaimSmartObject(UBehaviorTreeComponent* BTComponent, USmartObjectSubsystem* SmartObjectSubsystem,
						  FSmartObjectClaimHandle ClaimHandle);
};
