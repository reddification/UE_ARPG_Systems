#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Base.h"

FString FBehaviorEvaluatorOperation_Base::GetShortDescription() const
{
	FString OperationString = StaticEnum<EBehaviorEvaluatorOperationType>()->GetDisplayValueAsText(OperationType).ToString();
	FString OperationDescriptionFragment = FString::Printf(TEXT("[%s]"), *GetShortDescriptionInternal()) 
		+ (OperationDescription.IsEmpty() ? TEXT("") : FString::Printf(TEXT(" [%s]"), *OperationDescription));
	FString StateFilterInfo = RequiredState.IsSet() 
		? FString::Printf(TEXT(" [only when %s]"), *StaticEnum<EBehaviorEvaluatorState>()->GetDisplayValueAsText(RequiredState.GetValue()).ToString())
		: TEXT("");
	FString ScaleInfo = !FMath::IsNearlyEqual(ConstScale,1.f, 0.0001f) 
		? *FString::Printf(TEXT(" [x%.2f]"), ConstScale)
		: TEXT(""); 
	FString CachedVariableInfo = bCacheResult 
		? FString::Printf(TEXT(" [SET Result -> %s]"), *CachedResultName.ToString()) 
		: TEXT("");
	
	return FString::Printf(TEXT("%s %s%s%s%s "), *OperationString, *OperationDescriptionFragment, *StateFilterInfo, *ScaleInfo, *CachedVariableInfo);
}

FString FBehaviorEvaluatorOperation_Base::GenerateFormulaDescription(int Indentation) const
{
	FString OperatorString = GetOperatorString();
	FString OperationDescriptionFragment = OperationDescription.IsEmpty() ? TEXT("") : FString::Printf(TEXT(" [%s]"), *OperationDescription);
	FString StateFilterInfo = RequiredState.IsSet() 
		? FString::Printf(TEXT(" [only when %s]"), *StaticEnum<EBehaviorEvaluatorState>()->GetDisplayValueAsText(RequiredState.GetValue()).ToString())
		: TEXT("");
	FString ScaleInfo = !FMath::IsNearlyEqual(ConstScale,1.f, 0.0001f) 
		? *FString::Printf(TEXT(" [x%.2f]"), ConstScale)
		: TEXT(""); 
	FString IndentationStr = FString::ChrN(Indentation, TEXT('\t'));
	FString CachedVariableInfo = bCacheResult 
		? FString::Printf(TEXT(" [SET Result -> %s]"), *CachedResultName.ToString()) 
		: TEXT("");
	
	return FString::Printf(TEXT("%s%s%s%s%s%s "),
		*IndentationStr, *OperatorString, *OperationDescriptionFragment, *StateFilterInfo, *ScaleInfo, *CachedVariableInfo);
}

FString FBehaviorEvaluatorOperation_Base::GetOperatorString() const
{
	FString OperatorString = TEXT("");
	switch (OperationType) {
	case EBehaviorEvaluatorOperationType::Add:
		OperatorString = TEXT(" + ");
		break;
	case EBehaviorEvaluatorOperationType::Multiply:
		OperatorString = TEXT(" * ");
		break;
	case EBehaviorEvaluatorOperationType::Divide:
		OperatorString = TEXT(" / ");
		break;
	case EBehaviorEvaluatorOperationType::Subtract:
		OperatorString = TEXT(" - ");
		break;
	case EBehaviorEvaluatorOperationType::Power:
		OperatorString = TEXT("^");
		break;
	default:
		OperatorString = TEXT(" NOP ");
		break;
	}
	
	return OperatorString;
}
