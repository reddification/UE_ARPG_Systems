// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "Interfaces/NpcGoalParametersProvider.h"
#include "FlowNode_NpcGoal_UseSmartObject.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: Use smart object"))
class ARPGAI_API UFlowNode_NpcGoal_UseSmartObject : public UFlowNode_NpcGoal, public INpcGoalParametersProvider
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_UseSmartObject(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::UseSmartObject; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcGoalDataParameterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="NpcGoalDataParameterId.IsValid() == false"))
	FNpcGoalParameters_UseSmartObject DefaultParameters;

	virtual ENpcGoalStartResult Start() override;
	virtual ENpcGoalStartResult Restore(bool bInitialStart) override;
	virtual ENpcGoalAdvanceResult Advance(const FGameplayTagContainer& GoalExecutionResultTags) override;
	virtual void Suspend() override;
	virtual void ClearBlackboard() override;
	
	const FNpcGoalParameters_UseSmartObject* GetParameters() const;

#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetGoalDescription() const override;
#endif

public: // INpcGoalParametersProvider
	virtual struct FConstStructView GetParametersView() override;
};
