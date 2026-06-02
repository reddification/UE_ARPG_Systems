#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluatorOperations_Base.h"
#include "BehaviorEvaluatorOperations_Conditions.h"
#include "GameplayTagContainer.h"
#include "Data/NpcMemoryDataTypes.h"
#include "BehaviorEvaluatorOperations_Aggregating.generated.h"

class UNpcPerceptionComponent;
struct FCharacterPerceptionData;

USTRUCT(BlueprintType)
struct ARPGAI_API FBehaviorEvaluatorOperation_Aggregating_Base : public FBehaviorEvaluatorOperation_Base
{
	GENERATED_BODY()
	
public:
	float Evaluate(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent, float CurrentScore) const;
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const { return 0.0f; };
};

USTRUCT(BlueprintType, DisplayName="Scalar | Owner tags")
struct FBehaviorEvaluatorOperation_OwnerTags : public FBehaviorEvaluatorOperation_Aggregating_Base
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorOperation_Aggregating_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("Tags query scalars [%.2f | %.2f]"), ValuePositive, FallbackValue); }
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	virtual FString GetShortDescriptionInternal() const override 
	{ return FString::Printf(TEXT("Scalar: owner tags [%.2f | %.2f]"), ValuePositive, FallbackValue); };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery TagQuery;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ValuePositive = 0.f;
};

USTRUCT(BlueprintType, DisplayName="Scalar | Cached Variable")
struct FBehaviorEvaluatorOperation_Aggregating_CachedVariable : public FBehaviorEvaluatorOperation_Aggregating_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Aggregating_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("GET [%s] | %.2f"), *CachedVariable.ToString(), FallbackValue); };
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CachedVariable = FName("variable1");
	
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override
	{ return Context.CachedVariables.Contains(CachedVariable) ? Context.CachedVariables[CachedVariable] : FallbackValue; };
	
	virtual FString GetShortDescriptionInternal() const override 
	{ return FString::Printf(TEXT("GET [%s] | %.2f"), *CachedVariable.ToString(), FallbackValue); };
};

USTRUCT(BlueprintType, meta=(Hidden))
struct FBehaviorEvaluatorOperation_Aggregating_NonLinear : public FBehaviorEvaluatorOperation_Aggregating_Base
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorOperation_Aggregating_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override { return Super::GenerateFormulaDescription(Indentation) + TEXT("[NLD] "); };
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DependencyCurve;
};

USTRUCT(BlueprintType, DisplayName="NonLinear|Total accumulated damage from enemies")
struct FBehaviorEvaluatorOperation_TotalAccumulatedDamageFromEnemies : public FBehaviorEvaluatorOperation_Aggregating_NonLinear
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorOperation_Aggregating_NonLinear;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
		{ return Super::GenerateFormulaDescription(Indentation) + TEXT("Total accumulated damage from enemies"); };
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;	
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Total accumulated damage from enemies"); };
};

USTRUCT(BlueprintType, DisplayName="NonLinear|Normalized health")
struct FBehaviorEvaluatorOperation_NormalizedHealth : public FBehaviorEvaluatorOperation_Aggregating_NonLinear
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorOperation_Aggregating_NonLinear;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
		{ return Super::GenerateFormulaDescription(Indentation) + TEXT("Normalized health"); };
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;	
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Normalized health"); };
};

USTRUCT(BlueprintType, DisplayName="NonLinear|Behavior duration")
struct FBehaviorEvaluatorOperation_Aggregating_BehaviorDuration : public FBehaviorEvaluatorOperation_Aggregating_NonLinear
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorOperation_Aggregating_NonLinear;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("Behavior Duration"); };
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;	
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Behavior Duration"); };
};

USTRUCT(BlueprintType, DisplayName="NonLinear|Count of characters")
struct FBehaviorEvaluatorOperation_CountOfCharacters : public FBehaviorEvaluatorOperation_Aggregating_NonLinear
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorOperation_Aggregating_NonLinear;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("# of filtered characters"); }
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Count of characters"); };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float MaxDistance = 2000.f;
	
	// can be empty
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer AttitudeTagOptions;
	
	// can be empty
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ActorFilter;
	
	// can be empty
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDetectionSource DetectionSourceMask = EDetectionSource::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDetectionSourceMaskStrict = true;
};

USTRUCT(BlueprintType, DisplayName="NonLinear|Count of alive allies")
struct FBehaviorEvaluatorOperation_CountOfAliveAllies : public FBehaviorEvaluatorOperation_Aggregating_NonLinear
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorOperation_Aggregating_NonLinear;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("# of alive allies"); }
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Count of alive allies"); };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float MaxDistance = 2000.f;
};

USTRUCT(BlueprintType, DisplayName="Aggregation")
struct FBehaviorEvaluatorOperation_Aggregative_Aggregation : public FBehaviorEvaluatorOperation_Aggregating_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Aggregating_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	virtual FString GetShortDescriptionInternal() const override { return FString::Printf(TEXT("Operations aggregations [%d]"), Operations.Num()) ; };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FBehaviorEvaluatorOperation_Aggregating_Base>> Operations;
};

USTRUCT(BlueprintType, DisplayName="If-Else")
struct FBehaviorEvaluatorOperation_Aggregating_IfElse : public FBehaviorEvaluatorOperation_Aggregating_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Aggregating_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	virtual FString GetShortDescriptionInternal() const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FBehaviorEvaluatorOperationCondition_Base> Condition;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FBehaviorEvaluatorOperation_Aggregating_Base> IfTrue;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FBehaviorEvaluatorOperation_Aggregating_Base> IfFalse;
};

USTRUCT(BlueprintType)
struct FBehavorEvaluatorAggregatingOperationsContainer : public FBehavorEvaluatorOperationsContainerBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FBehaviorEvaluatorOperation_Aggregating_Base>> Operations;
	
	virtual void GenerateFormulaDescription() override;
};
