#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RememberActor.generated.h"

UCLASS(Category="Logic")
class ARPGAI_API UBTTask_RememberActor : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_RememberActor();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ActorKey;
	
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer RememberTraits;
	
	UPROPERTY(EditAnywhere)
	bool bRememberForever = true;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="!bRememberForever", UIMin = 0.f, ClampMin = 0.f))
	float RememberForDurationGTH = 0.f;
};
