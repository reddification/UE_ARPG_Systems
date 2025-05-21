// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Data/NpcReactionsDataAsset.h"
#include "BTTask_CompleteReaction.generated.h"

enum class EReactionBehaviorType : uint8;
/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_CompleteReaction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CompleteReaction();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere)
	EReactionBehaviorType ReactionBehaviorType = EReactionBehaviorType::None;
};
