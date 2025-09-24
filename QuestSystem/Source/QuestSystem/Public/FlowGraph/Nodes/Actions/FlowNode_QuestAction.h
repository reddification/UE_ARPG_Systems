// 

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/DelayedQuestActionInterface.h"
#include "Nodes/FlowNode.h"
#include "AddOns/FlowNodeAddOn.h"

#include "FlowNode_QuestAction.generated.h"

struct FQuestSystemContext;
/**
 * 
 */

enum EQuestActionExecuteResult : uint8
{
	Success,
	Failure,
	Latent
};

UCLASS(Abstract, NotBlueprintable, Hidden)
class QUESTSYSTEM_API UFlowNode_QuestAction : public UFlowNode, public IDelayedQuestAction
{
	GENERATED_BODY()

protected:

public:
	UFlowNode_QuestAction(const FObjectInitializer& ObjectInitializer);
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
public:
	virtual FText GetNodeConfigText() const override;
protected:
	virtual FString GetQuestActionDescription() const;
#endif
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString UserDescription;

	// Must be persistent (hence not re-generated in between game starts)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGuid ActionId;
	
	// If both set, StartAtNextTimeOfDay has a priority over GameTimeDelayHours
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ActionId.IsValid()"))
	FGameplayTag StartAtNextTimeOfDay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="ActionId.IsValid()"))
	float GameTimeDelayHours = 0.f;
	
	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnChild_Implementation(const UFlowNodeAddOn* AddOnTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;
	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context);
	virtual void OnLatentActionFinished(EQuestActionExecuteResult ExecutionResult);
	
private:
	FORCEINLINE bool IsDelayed() const { return GameTimeDelayHours > 0.f || StartAtNextTimeOfDay.IsValid(); }

	FName Execute(const FQuestSystemContext& Context);
	bool CanExecute(const FQuestSystemContext& Context) const;

	static FName PinOut_Executed;
	static FName PinOut_Failed;
	static FName PinOut_Delayed;
	static FName PinOut_CannotExecute;
	
public: // IDelayedQuestAction
	virtual void StartDelayedAction(const FQuestSystemContext& QuestSystemContext) override;

};
