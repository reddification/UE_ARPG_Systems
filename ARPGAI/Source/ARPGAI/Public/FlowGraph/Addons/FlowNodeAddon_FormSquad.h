// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_ActivityParameters.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Addons/FlowNodeAddOnStateful.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "FlowNodeAddon_FormSquad.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UFlowNodeAddon_FormSquad : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void FinishState() override;
	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnParent_Implementation(const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMIn = 1, ClampMin = 1))
	int DesiredFollowers = 1;

	// what to start in their flow graph
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag FollowerActivityId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer SuitableSquadMembersIds;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery SuitableSquadMembersFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SquadMemberAttitudePreset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcSquadMemberFollowParameters SquadMemberParameters;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<class UNpcReactionEvaluatorBase*> PerceptionReactionEvaluators;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FollowersInRange = 5000.f;

private:
	bool bSquadCreated;
};
