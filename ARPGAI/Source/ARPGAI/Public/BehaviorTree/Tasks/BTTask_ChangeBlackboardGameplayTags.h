// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChangeBlackboardGameplayTags.generated.h"

UENUM()
enum class EChangeBlackboardGameplayTagsType : uint8
{
	Append,
	Overwrite,
	Remove,
};

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_ChangeBlackboardGameplayTags : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ChangeBlackboardGameplayTags();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector GameplayTagsBBKey;

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer DeltaTags;

	UPROPERTY(EditAnywhere)
	EChangeBlackboardGameplayTagsType ChangeBlackboardGameplayTagsType;
};
