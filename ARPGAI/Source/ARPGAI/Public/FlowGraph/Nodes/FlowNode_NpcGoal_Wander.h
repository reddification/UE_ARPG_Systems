// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "FlowNode_NpcGoal_Wander.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: Wander"))
class ARPGAI_API UFlowNode_NpcGoal_Wander : public UFlowNode_NpcGoal
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_Wander(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::Wander; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UEnvQuery* AreaEqs;

	UPROPERTY(Category = EQS, EditAnywhere)
	FEQSParametrizedQueryExecutionRequest EQSRequest_Test;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer GestureOptionsTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer SpeechOptionsTags;
	
	virtual ENpcGoalStartResult Restore(bool bInitialStart) override;

#if WITH_EDITOR
public:
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetGoalDescription() const override;
#endif
};
