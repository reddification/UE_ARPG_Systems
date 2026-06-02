#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluatorOperations_DataTypes.h"
#include "Data/BehaviorEvaluatorDataTypes.h"
#include "BehaviorEvaluatorOperations_Base.generated.h"

USTRUCT(BlueprintType)
struct ARPGAI_API FBehaviorEvaluatorOperation_Base
{
	GENERATED_BODY()

public:
	virtual ~FBehaviorEvaluatorOperation_Base() = default;
	
	virtual FString GetShortDescription() const;
	virtual FString GenerateFormulaDescription(int Indentation) const;
	FORCEINLINE bool IsEnabled() const { return bEnabled; }
	
protected:
	virtual FString GetShortDescriptionInternal() const { return TEXT("Base"); };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBehaviorEvaluatorOperationType OperationType = EBehaviorEvaluatorOperationType::Add;
	
	// Used in auto generated formula and logging. Recommended to be laconic
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString OperationDescription;
	
	// returned whenever evaluation is impossible (absence of data, etc)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FallbackValue = 0.f;
	
	// scale to apply to this operation whatever the result is
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ConstScale = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (InlineEditConditionToggle))
	bool bCacheResult = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bCacheResult"))
	FName CachedResultName = FName("variable1");	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnabled = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TOptional<EBehaviorEvaluatorState> RequiredState;
	
	FString GetOperatorString() const;
};
