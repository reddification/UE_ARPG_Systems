// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "NpcGoalBackgroundTasks.generated.h"

class UNpcActivityComponent;

UCLASS(Abstract, EditInlineNew)
class ARPGAI_API UNpcGoalBackgroundTaskBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void Start(UNpcActivityComponent* NpcActivityComponent, bool bFirstStart) { };

	virtual void Stop(UNpcActivityComponent* NpcActivityComponent) { };

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bStartWithGoal = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition = "bStartWithGoal == false"))
	FGameplayTag StartAfterGoalAdvanceTag;
};

UCLASS()
class ARPGAI_API UNpcGoalBackgroundTaskRealtimeDialogue : public UNpcGoalBackgroundTaskBase
{
	GENERATED_BODY()

public:
	virtual void Start(UNpcActivityComponent* NpcActivityComponent, bool bFirstStart) override;
	
	virtual void Stop(UNpcActivityComponent* NpcActivityComponent) override;

protected:
	// Currently will pick closest NPCs with these tags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ConversationPartnersIds;

	// Currently will pick closest NPCs with these tags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ConversationId;

private:
	void StartConversation(UNpcActivityComponent* NpcActivityComponent, bool bResume) const;
};
