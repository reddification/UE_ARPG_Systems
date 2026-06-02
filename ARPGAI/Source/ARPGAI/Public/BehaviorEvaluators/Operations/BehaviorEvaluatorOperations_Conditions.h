#pragma once

#include "CoreMinimal.h"
#include "Data/BehaviorEvaluatorDataTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "BehaviorEvaluatorOperations_Conditions.generated.h"

struct FRelativeOperationContext;
struct FAggregationOperationContext;
struct FCharacterPerceptionData;
class UNpcPerceptionComponent;

USTRUCT(BlueprintType)
struct FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual ~FBehaviorEvaluatorOperationCondition_Base() = default;
	
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterPerceptionData& CharacterPerceptionData) const { return true; }
	virtual bool Evaluate(const FAggregationOperationContext& Context,
		const UNpcPerceptionComponent* NpcPerceptionComponent) const { return true; }

	virtual FString ToString(int Indentation) const { return FString::ChrN(Indentation, '\t'); }
};

USTRUCT(DisplayName="Binary logical operation", meta=(Hidden))
struct FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperationCondition_Base;
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterPerceptionData& CharacterPerceptionData) const override;
	virtual bool Evaluate(const FAggregationOperationContext& Context,
		const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TInstancedStruct<FBehaviorEvaluatorOperationCondition_Base> Statement1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TInstancedStruct<FBehaviorEvaluatorOperationCondition_Base> Statement2;
	
	virtual FString ToString(int Indentation) const override;
	
protected:
	virtual bool EvaluateInternal(bool bStatement1, bool bStatement2) const { return false; };
	virtual FString BinaryOpInfo() const { return TEXT("Binary"); }
};

using FConditionBinary = FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary;

USTRUCT(DisplayName="Logical | AND")
struct FBehaviorEvaluatorOperationCondition_LogicalOperation_Conjunction : public FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary
{
	GENERATED_BODY()
	
	using Super = FConditionBinary;
	
protected:
	virtual bool EvaluateInternal(bool bStatement1, bool bStatement2) const override;
	virtual FString BinaryOpInfo() const override { return TEXT("AND"); }
};

USTRUCT(DisplayName="Logical | OR")
struct FBehaviorEvaluatorOperationCondition_LogicalOperation_Disjunction : public FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary
{
	GENERATED_BODY()
	
	using Super = FConditionBinary;
	
protected:
	virtual bool EvaluateInternal(bool bStatement1, bool bStatement2) const override;
	virtual FString BinaryOpInfo() const override { return TEXT("OR"); }
};

USTRUCT(DisplayName="Logical | NOT")
struct FBehaviorEvaluatorOperationCondition_Unary_Not : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperationCondition_Base;
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterPerceptionData& CharacterPerceptionData) const override;
	virtual bool Evaluate(const FAggregationOperationContext& Context,
		const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	
	virtual FString ToString(int Indentation) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TInstancedStruct<FBehaviorEvaluatorOperationCondition_Base> Statement1;
};

USTRUCT(DisplayName="Evaluator state")
struct FBehaviorEvaluatorOperationCondition_EvaluatorState : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterPerceptionData& CharacterPerceptionData) const override;
	virtual bool Evaluate(const FAggregationOperationContext& Context,
		const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	
	virtual FString ToString(int Indentation) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBehaviorEvaluatorState RequiredState = EBehaviorEvaluatorState::Activated;
};

USTRUCT(DisplayName="Activation | Visual Contact Duration")
struct FBehaviorEvaluatorOperationCondition_Activation_VisualContactDuration : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterPerceptionData& CharacterPerceptionData) const override;
	virtual bool Evaluate(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	
	virtual FString ToString(int Indentation) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ActivationThreshold = 0.5f;
};

USTRUCT(DisplayName="Activation | Accumulated score")
struct FBehaviorEvaluatorOperationCondition_Activation_AccumulatedScore : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterPerceptionData& CharacterPerceptionData) const override;
	virtual bool Evaluate(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	
	virtual FString ToString(int Indentation) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ActivationThreshold = 0.5f;
};

USTRUCT(DisplayName="Activation | Behavior Duration")
struct FBehaviorEvaluatorOperationCondition_Activation_BehaviorDuration : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterPerceptionData& CharacterPerceptionData) const override;
	virtual bool Evaluate(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	
	virtual FString ToString(int Indentation) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ActivationThreshold = 0.5f;
};

