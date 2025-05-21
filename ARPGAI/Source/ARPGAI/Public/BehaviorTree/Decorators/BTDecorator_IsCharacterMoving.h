// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsCharacterMoving.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_IsCharacterMoving : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsCharacterMoving();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector CharacterBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TickInterval = 0.5f;
};
