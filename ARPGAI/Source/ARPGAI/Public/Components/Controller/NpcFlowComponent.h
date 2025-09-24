// 

#pragma once

#include "CoreMinimal.h"
#include "FlowComponent.h"
#include "Data/AiDataTypes.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "FlowGraph/Nodes/FlowNode_NpcGoal.h"
#include "Interfaces/NpcGoalParametersProvider.h"
#include "NpcFlowComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcFlowComponent : public UFlowComponent
{
	GENERATED_BODY()

private:
	friend UFlowNode_NpcGoal;

	struct FNpcActivity
	{
		FGameplayTag ActivityTag;
		TWeakObjectPtr<UFlowNode_NpcGoal> ActiveGoalNode; // not necessarily "active"-active. it can be suspended, but this is the last goal of the quest activity
	};
	
	DECLARE_DELEGATE_RetVal_OneParam(ENpcGoalAdvanceResult, FNpcGoalAdvanceEvent, const FGameplayTagContainer& GoalExecutionResultTags);
	DECLARE_DELEGATE_TwoParams(FNpcLocationCrossedEvent, const FGameplayTag& LocationId, bool bInside);
	DECLARE_DELEGATE(FNpcGoalControlEvent);
	
public:
	UNpcFlowComponent(const FObjectInitializer& ObjectInitializer);
	virtual void InitializeComponent() override;
	
	void SetDayTime(const FGameplayTag& DayTime);
	void RequestQuestActivity(const FGameplayTag& QuestActivityId);
	void StopQuestActivity(const FGameplayTag& QuestActivityId);
	
	void PauseGoal();
	void ResumeGoal();
	void ExternalCompleteGoal();
	void FinishActivity();
	
	ENpcGoalAdvanceResult AdvanceGoal(const FGameplayTagContainer& GoalExecutionResult);
	void OnNpcReachedLocation(const FGameplayTag& LocationIdTag);
	void OnNpcLeftLocation(const FGameplayTag& LocationId);
	
	void SetGoalTagParameter(const FGameplayTag& ParameterId, const FGameplayTag& ParameterValue);
	const FGameplayTag& GetGoalTagParameter(const FGameplayTag& ParameterId) const;
	void RemoveGoalTagParameter(const FGameplayTag& ParameterId);

	mutable FNpcLocationCrossedEvent NpcLocationCrossedEvent;

	virtual void OnGoalCompleted(const FGameplayTagContainer& GoalTags, const FGameplayTagContainer& GoalExecutionResultTags) {};

	template<typename T>
	const T* GetParameters()
	{
		if (!ActivitiesStack.IsEmpty())
		{
			if (auto ParametersProvider = Cast<INpcGoalParametersProvider>(ActivitiesStack.Last().ActiveGoalNode.Get()))
			{
				auto Parameters = ParametersProvider->GetParametersView();
				if (Parameters.IsValid())
					return Parameters.GetPtr<const T>();
			}
		}
		
		return nullptr;
	}
	
protected:
	TArray<FNpcActivity> ActivitiesStack;

private:
	void StartRoutineActivity();
	void OnGoalStarted(UFlowNode_NpcGoal* NpcGoal);
	void OnGoalEnded(UFlowNode_NpcGoal* NpcGoal);
	
	FGameplayTag CurrentDayTime;
	FGameplayTag LifecycleActivitySuspendedAtDayTime;

	
	TMap<FGameplayTag, FGameplayTag> GoalTagParameters;
};
