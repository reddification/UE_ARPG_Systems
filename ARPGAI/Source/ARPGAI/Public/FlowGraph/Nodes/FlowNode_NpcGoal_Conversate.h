// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "Interfaces/NpcGoalParametersProvider.h"
#include "FlowNode_NpcGoal_Conversate.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: Conversate"))
class ARPGAI_API UFlowNode_NpcGoal_Conversate : public UFlowNode_NpcGoal, public INpcGoalParametersProvider
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_Conversate(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::Conversate; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcGoalParameters_Conversate Parameters;

protected:
	virtual ENpcGoalStartResult Start() override;
	virtual ENpcGoalStartResult Restore(bool bInitialStart) override;

private:
	bool SetConversationBlackboardContext() const;

public: // INpcGoalParametersProvider
	virtual struct FConstStructView GetParametersView() override;

protected:
#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetGoalDescription() const override;
#endif
};
