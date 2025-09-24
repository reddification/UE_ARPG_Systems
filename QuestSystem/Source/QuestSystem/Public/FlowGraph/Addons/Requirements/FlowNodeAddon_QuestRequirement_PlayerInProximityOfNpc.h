// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestRequirement.h"
#include "FlowNodeAddon_QuestRequirement_PlayerInProximityOfNpc.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestRequirement_PlayerInProximityOfNpc : public UFlowNodeAddon_QuestRequirement
{
	GENERATED_BODY()

public:
	virtual bool EvaluatePredicate_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcId;

	// Visibility is traced from MainCharacter head to TestCharacter head
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bTraceVisibility = true;

	// Distance within which characters must be in order for the requirement to pass
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Proximity = 350.f;	
};
