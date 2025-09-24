// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestRequirement.h"
#include "Data/QuestRequirements.h"
#include "FlowNodeAddon_QuestRequirement_CharacterInventory.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestRequirement_CharacterInventory : public UFlowNodeAddon_QuestRequirement
{
	GENERATED_BODY()

public:
	virtual bool EvaluatePredicate_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FQuestRequirementItemFilter ItemFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bPlayer = true;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="!bPlayer"))
	FGameplayTag CharacterId;
};
