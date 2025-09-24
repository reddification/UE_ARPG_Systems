// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestRequirement.h"
#include "FlowNodeAddon_QuestRequirement_CharacterState.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestRequirement_CharacterState : public UFlowNodeAddon_QuestRequirement
{
	GENERATED_BODY()

public:
	virtual bool EvaluatePredicate_Implementation() const override;	
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery RequiresCharacterGameplayState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bPlayer = true;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="!bPlayer"))
	FGameplayTag CharacterId;
};
