// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "FlowNode_NpcGoal_Gesture.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: Gesture"))
class ARPGAI_API UFlowNode_NpcGoal_Gesture : public UFlowNode_NpcGoal
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_Gesture(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::Gesture; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer GestureOptions;

	virtual ENpcGoalStartResult Start() override;
	virtual ENpcGoalStartResult Restore(bool bInitialStart) override;
	
#if WITH_EDITOR
	virtual FString GetGoalDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
};
