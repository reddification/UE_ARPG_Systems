// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_ActivityParameters.h"
#include "AddOns/FlowNodeAddOn.h"
#include "FlowNodeAddon_DayTimeChange.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UFlowNodeAddon_DayTimeChange : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()

public:
	UFlowNodeAddon_DayTimeChange();
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FName> DayTimeTagToOutput;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual TArray<FFlowPin> GetContextOutputs() const override;
#endif
	
private:
	void OnDayTimeChanged(const FGameplayTag& NewDayTime);
	FDelegateHandle DayTimeChangedDelegate;
};
