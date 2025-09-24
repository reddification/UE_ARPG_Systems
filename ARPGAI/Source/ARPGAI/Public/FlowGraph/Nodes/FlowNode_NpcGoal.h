// 

#pragma once

#include "CoreMinimal.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "Nodes/FlowNode.h"
#include "Nodes/FlowNodeStateful.h"
#include "FlowNode_NpcGoal.generated.h"

class UBlackboardComponent;
class UNpcBlackboardDataAsset;
class UNpcComponent;
class AAIController;
class UNpcFlowComponent;
/**
 * 
 */
UCLASS(Abstract)
class ARPGAI_API UFlowNode_NpcGoal : public UFlowNodeStateful
{
	GENERATED_BODY()

protected:
	enum EGoalState
	{
		Inactive,
		Running,
		Suspended,
	};
	
public:
	UFlowNode_NpcGoal(const FObjectInitializer& ObjectInitializer);
	virtual void ExecuteInput(const FName& PinName) override;
	virtual void InitializeInstance() override;
	
#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetGoalDescription() const;
	virtual FText GetNodeConfigText() const override;
	virtual FString GetStatusString() const override;
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FString ExtraUserDescription;
#endif

	ENpcGoalAdvanceResult RequestAdvanceGoal(const FGameplayTagContainer& GoalExecutionResultTags);
	void RequestSuspendGoal();
	void RequestResumeGoal();
	void RequestExternalAbort();
	void RequestLoadState();
	
protected:
	virtual void ClearBlackboard();
	virtual ENpcGoalStartResult Start();
	virtual ENpcGoalStartResult Restore(bool bInitialStart);
	virtual ENpcGoalAdvanceResult Advance(const FGameplayTagContainer& GoalExecutionResultTags);
	virtual void Suspend();

	virtual FFlowDataPinResult_GameplayTagContainer TrySupplyDataPinAsGameplayTagContainer_Implementation(const FName& PinName) const override;
	
	UPROPERTY(EditAnywhere)
	bool bRunIndefinitely = false;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f, EditCondition="bRunIndefinitely == false"))
	float GameTimeDurationInHours = 1.f;

	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f, EditCondition="bRunIndefinitely == false"))
	float RandomTimeDeviation = 0.2f;

	// Do whatever you want with these. In G2VS2 these are used to notify quest system that some quest behavior goal has been completed
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer CustomGoalTags;

	virtual void Finish() override;

	ENpcGoalType NpcGoalType;

	TWeakObjectPtr<APawn> NpcPawn;
	TWeakObjectPtr<UNpcFlowComponent> NpcFlowComponent;
	TWeakObjectPtr<UNpcComponent> NpcComponent;
	TWeakObjectPtr<UBlackboardComponent> BlackboardComponent;
	
	UPROPERTY()
	TObjectPtr<UNpcBlackboardDataAsset> BlackboardKeys;

	UPROPERTY(EditAnywhere, Category = DataPins, DisplayName = "Goal execution result tags", meta = (SourceForOutputFlowPin, FlowPinType = "GameplayTagContainer"))
	FGameplayTagContainer OutGoalExecutionResultTags;
	EGoalState CurrentGoalState = EGoalState::Inactive;
	
private:
	float RemainingGoalExecutionTime = 0.f;
};
