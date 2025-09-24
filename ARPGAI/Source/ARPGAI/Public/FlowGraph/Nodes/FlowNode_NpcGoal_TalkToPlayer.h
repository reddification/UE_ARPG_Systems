// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "Interfaces/NpcGoalParametersProvider.h"
#include "FlowNode_NpcGoal_TalkToPlayer.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: Talk to player"))
class ARPGAI_API UFlowNode_NpcGoal_TalkToPlayer : public UFlowNode_NpcGoal, public INpcGoalParametersProvider
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_TalkToPlayer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::TalkToPlayer; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcGoalParameters_TalkToPlayer Parameters;
	
	virtual ENpcGoalStartResult Start() override;

#if WITH_EDITOR
	virtual FString GetGoalDescription() const override;
#endif
	
public: // INpcGoalParametersProvider
	virtual FConstStructView GetParametersView() override;
};
