#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_ActivityParameters.h"
#include "FlowNodeAddon_SetRoleplayObjective.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UFlowNodeAddon_SetRoleplayObjective : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ObjectiveTag;
	
private:
	FGameplayTag PreviousObjectiveTag;
};
