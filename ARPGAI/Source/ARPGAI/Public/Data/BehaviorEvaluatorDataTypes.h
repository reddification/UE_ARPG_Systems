#pragma once

#include "BehaviorEvaluatorDataTypes.generated.h"

class UNpcBehaviorEvaluatorComponent2;

UENUM(BlueprintType)
enum class EBehaviorEvaluatorState : uint8
{
	// This behavior utility is not updated. if behavior is active - resets the utility to zero
	NotRequested = 0,
	// Something has requested to block this evaluator. could pe indefinite, could be time limited. If behavior was active - resets its utility to zero
	Blocked,
	// This behavior utility is updated 
	Relevant, 
	// This is the currently active AI behavior
	Activated,
};

enum class EBehaviorEvaluatorResult : uint8
{
	Success,
	Fail,
	Abort,
	Remove
};

