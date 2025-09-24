// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestRequirement.h"
#include "FlowNodeAddon_QuestRequirement_WorldState.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestRequirement_WorldState : public UFlowNodeAddon_QuestRequirement
{
	GENERATED_BODY()

public:
	virtual bool EvaluatePredicate_Implementation() const override;

#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery WorldStateQuery;
};
