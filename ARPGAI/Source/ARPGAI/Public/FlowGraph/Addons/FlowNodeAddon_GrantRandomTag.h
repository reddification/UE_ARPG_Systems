// 

#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Addons/FlowNodeAddOnStateful.h"
#include "FlowNodeAddon_GrantRandomTag.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UFlowNodeAddon_GrantRandomTag : public UFlowNodeAddOnStateful
{
	GENERATED_BODY()
	
public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;
	
	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnParent_Implementation(const UFlowNodeBase* ParentTemplate,
		const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Tags;
	
private:
	FGameplayTag GrantedTag;
};
