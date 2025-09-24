// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_NpcStartRealtimeDialogue.generated.h"

struct FDialogueParticipantData;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_NpcStartRealtimeDialogue : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	// Dialogue instigator
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag DialogueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDialogueParticipantData> DialogueParticipants;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIncludePlayer = true;
	
	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
