// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_LoadReactionBehaviorContext.generated.h"

enum class EReactionBehaviorType : uint8;
/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_LoadReactionBehaviorContext : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_LoadReactionBehaviorContext();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EReactionBehaviorType ReactionBehaviorType;
};
