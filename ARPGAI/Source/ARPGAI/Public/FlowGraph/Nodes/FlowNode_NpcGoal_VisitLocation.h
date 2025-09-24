// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "FlowNode_NpcGoal_VisitLocation.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: Visit location"))
class ARPGAI_API UFlowNode_NpcGoal_VisitLocation : public UFlowNode_NpcGoal
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_VisitLocation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::VisitLocation; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Location.Id,G2VS2.Location.Id"))
	FGameplayTagContainer LocationOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCompleteOnEntering = false;

	virtual ENpcGoalStartResult Start() override;
	virtual void Suspend() override;
	virtual void Finish() override;
	
#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetGoalDescription() const override;
#endif
	
private:
	FGameplayTag SelectedLocationId;
	void OnLocationCrossed(const FGameplayTag& CrossedLocationId, bool bEntered);
};
