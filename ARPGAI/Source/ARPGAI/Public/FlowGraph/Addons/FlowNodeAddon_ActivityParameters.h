// 

#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Addons/FlowNodeAddOnStateful.h"
#include "UObject/SoftObjectPtr.h"
#include "FlowNodeAddon_ActivityParameters.generated.h"

class UNavigationQueryFilter;

/**
 * 
 */
UCLASS()
class ARPGAI_API UFlowNodeAddon_ActivityParameters : public UFlowNodeAddOnStateful
{
	GENERATED_BODY()

public:
	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnParent_Implementation(const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;
};

UCLASS()
class ARPGAI_API UFlowNodeAddon_SetActivityLocations : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer ActivityLocations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOverwriteActivityLocations = false;

#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
};

UCLASS()
class ARPGAI_API UFlowNodeAddon_SetCharacterState : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterStateTag;
	
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;
};

UCLASS()
class ARPGAI_API UFlowNodeAddon_SetAttitudePreset : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AttitudePresetTag;
	
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;
};

UCLASS()
class ARPGAI_API UFlowNodeAddon_SetNavigationFilterClass : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UNavigationQueryFilter> NavigationFilterClass;
	
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;

private:
	TSubclassOf<UNavigationQueryFilter> InitialNavigationFilterClass;
};

UCLASS()
class ARPGAI_API UFlowNodeAddon_RequestBehaviorEvaluators : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer BehaviorEvaluatorsTags;

	// if false -> request blocked
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bRequestActive = true;
	
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;

#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
};