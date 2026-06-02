#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "BTTask_Gesture.generated.h"

UCLASS(Category="Emotes")
class ARPGAI_API UBTTask_Gesture : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_Gesture();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAwaitCompletion = true;
	
	// Obsolete
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayTag GestureActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer GestureOptions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ActivityGestureBBKey;
	
private:
	FGameplayTag DetermineActualGestureTag(UBehaviorTreeComponent& OwnerComp) const;
};
