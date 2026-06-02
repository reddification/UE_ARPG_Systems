#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Conditions.h"

#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_DataTypes.h"
#include "Components/Controller/NpcPerceptionComponent.h"

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary::Evaluate(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterPerceptionData& CharacterPerceptionData) const
{
	auto Base = Super::Evaluate(Context, Target, CharacterPerceptionData);
	if (!Base)
		return false;
	
	if (!Statement1.IsValid() || !Statement2.IsValid())
		return false;
	
	const bool b1 = Statement1.Get().Evaluate(Context, Target, CharacterPerceptionData);
	const bool b2 = Statement2.Get().Evaluate(Context, Target, CharacterPerceptionData);
	return EvaluateInternal(b1, b2);
}

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary::Evaluate(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	auto Base = Super::Evaluate(Context, NpcPerceptionComponent);
	if (!Base)
		return false;
	
	if (!Statement1.IsValid() || !Statement2.IsValid())
		return false;
	
	const bool b1 = Statement1.Get().Evaluate(Context, NpcPerceptionComponent);
	const bool b2 = Statement2.Get().Evaluate(Context, NpcPerceptionComponent);
	return EvaluateInternal(b1, b2);
}

FString FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary::ToString(int Indentation) const
{
	auto Base = Super::ToString(Indentation);
	if (!Statement1.IsValid())
		return Base + TEXT("Error! Statement 1 is not set");
	
	if (!Statement2.IsValid())
		return Base + TEXT("Error! Statement 2 is not set");
	
	FString Indentation1Str = FString::ChrN(Indentation + 2, '\t');
	return Base + FString::Printf(TEXT("%s\n%s%s\n%s"), 
		*Statement1.Get().ToString(Indentation + 1), *Indentation1Str, *BinaryOpInfo(), *Statement2.Get().ToString(Indentation + 1));
}

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Conjunction::EvaluateInternal(bool bStatement1, bool bStatement2) const
{
	return bStatement1 && bStatement2;
}

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Disjunction::EvaluateInternal(bool bStatement1,
	bool bStatement2) const
{
	return bStatement1 || bStatement2;
}

bool FBehaviorEvaluatorOperationCondition_Unary_Not::Evaluate(const FRelativeOperationContext& Context,
	const AActor* Target, const FCharacterPerceptionData& CharacterPerceptionData) const
{
	auto Base = Super::Evaluate(Context, Target, CharacterPerceptionData);
	if (!Base)
		return false;
	
	if (!Statement1.IsValid())
		return false;
	
	return !Statement1.Get().Evaluate(Context, Target, CharacterPerceptionData);
}

bool FBehaviorEvaluatorOperationCondition_Unary_Not::Evaluate(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	auto Base = Super::Evaluate(Context, NpcPerceptionComponent);
	if (!Base)
		return false;
	
	if (!Statement1.IsValid())
		return false;
	
	return !Statement1.Get().Evaluate(Context, NpcPerceptionComponent);
}

FString FBehaviorEvaluatorOperationCondition_Unary_Not::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("NOT %s"), *Statement1.Get().ToString(Indentation + 1));
}

bool FBehaviorEvaluatorOperationCondition_EvaluatorState::Evaluate(const FRelativeOperationContext& Context,
                                                                           const AActor* Target, const FCharacterPerceptionData& CharacterPerceptionData) const
{
	return Super::Evaluate(Context, Target, CharacterPerceptionData) && Context.EvaluatorState == RequiredState;
}

bool FBehaviorEvaluatorOperationCondition_EvaluatorState::Evaluate(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return Super::Evaluate(Context, NpcPerceptionComponent) && Context.EvaluatorState == RequiredState;
}

FString FBehaviorEvaluatorOperationCondition_EvaluatorState::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("Evaluator state == %s"), 
		*StaticEnum<EBehaviorEvaluatorState>()->GetDisplayValueAsText(RequiredState).ToString());
}

bool FBehaviorEvaluatorOperationCondition_Activation_VisualContactDuration::Evaluate(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterPerceptionData& CharacterPerceptionData) const
{
	return Super::Evaluate(Context, Target, CharacterPerceptionData)
		&& CharacterPerceptionData.HasVisualDetection() && CharacterPerceptionData.TimeSeen >= ActivationThreshold;
}

bool FBehaviorEvaluatorOperationCondition_Activation_VisualContactDuration::Evaluate(
	const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	unimplemented();
	return Super::Evaluate(Context, NpcPerceptionComponent);
}

FString FBehaviorEvaluatorOperationCondition_Activation_VisualContactDuration::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("See target longer than %.2fs"), ActivationThreshold);
}

bool FBehaviorEvaluatorOperationCondition_Activation_AccumulatedScore::Evaluate(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterPerceptionData& CharacterPerceptionData) const
{
	return Context.AccumulatedScore >= ActivationThreshold;
}

bool FBehaviorEvaluatorOperationCondition_Activation_AccumulatedScore::Evaluate(
	const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return Context.AccumulatedScore >= ActivationThreshold;
}

FString FBehaviorEvaluatorOperationCondition_Activation_AccumulatedScore::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("Accumulated score >= %.2fs"), ActivationThreshold);
}

bool FBehaviorEvaluatorOperationCondition_Activation_BehaviorDuration::Evaluate(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterPerceptionData& CharacterPerceptionData) const
{
	return Context.ActiveBehaviorDuration >= ActivationThreshold;
}

bool FBehaviorEvaluatorOperationCondition_Activation_BehaviorDuration::Evaluate(
	const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return Context.ActiveBehaviorDuration >= ActivationThreshold;
}

FString FBehaviorEvaluatorOperationCondition_Activation_BehaviorDuration::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("Behavior duration >= %.2fs"), ActivationThreshold);
}
