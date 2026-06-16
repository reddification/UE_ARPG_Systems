#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/BehaviorEvaluatorDataTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "BehaviorEvaluatorOperations_Conditions.generated.h"

struct FRelativeOperationContext;
struct FAggregationOperationContext;
struct FCharacterShortTermMemory;
class UNpcPerceptionComponent;

USTRUCT(BlueprintType)
struct FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual ~FBehaviorEvaluatorOperationCondition_Base() = default;
	
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	                      const FCharacterShortTermMemory& CharacterSTM) const { return true; }
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
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
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

UENUM()
enum class EBehaviorEvaluatorConditionCompoundOperation : uint8
{
	AND,
	OR,
};

USTRUCT(DisplayName="Logical | Compound")
struct FBehaviorEvaluatorOperationCondition_LogicalOperation_Compound : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperationCondition_Base;
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterSTM) const override;
	virtual bool Evaluate(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBehaviorEvaluatorConditionCompoundOperation Mode = EBehaviorEvaluatorConditionCompoundOperation::AND;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FBehaviorEvaluatorOperationCondition_Base>> Statements;
	
	virtual FString ToString(int Indentation) const override;
	
protected:
	virtual FString BinaryOpInfo() const { return TEXT("Compound"); }
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
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
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
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
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
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
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
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
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
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
	virtual bool Evaluate(const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const override;
	
	virtual FString ToString(int Indentation) const override
	{ return Super::ToString(Indentation) + FString::Printf(TEXT("Behavior duration >= %.2fs"), ActivationThreshold); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ActivationThreshold = 0.5f;
};

// 
USTRUCT(DisplayName="Activation | In range")
struct FBehaviorEvaluatorOperationCondition_Activation_Distance : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
	
	virtual FString ToString(int Indentation) const override
	{ return Super::ToString(Indentation) + FString::Printf(TEXT("Distance <= %.2f"), DistanceThreshold); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float DistanceThreshold = 1000.f;
};

USTRUCT(DisplayName="Activation | Is alive")
struct FBehaviorEvaluatorOperationCondition_IsAlive : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
	
	virtual FString ToString(int Indentation) const override
	{ return Super::ToString(Indentation) + (bDesiredState ? TEXT("Is alive?") : TEXT("Is dead?")); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDesiredState = true;
};

USTRUCT(DisplayName="Activation | Is hostile")
struct FBehaviorEvaluatorOperationCondition_Activation_IsHostile : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	                      const FCharacterShortTermMemory& CharacterSTM) const override;
	
	virtual FString ToString(int Indentation) const override
	{ return Super::ToString(Indentation) + (bDesiredState ? TEXT("Is hostile?") : TEXT("Is NOT hostile?")); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	bool bDesiredState = true;
};

USTRUCT(DisplayName="Activation | In combat with allies")
struct FBehaviorEvaluatorOperationCondition_InCombatWithAllies : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterSTM) const override;
	
	virtual FString ToString(int Indentation) const override
	{ return Super::ToString(Indentation) + (bDesiredState ? TEXT("Is in combat with allies?") : TEXT("Is NOT in combat with allies?")); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDesiredState = true;
};

USTRUCT(DisplayName="Activation | Long term accumulated damage")
struct FBehaviorEvaluatorOperationCondition_LongTermAccumulatedDamage : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterSTM) const override;
	
	virtual FString ToString(int Indentation) const override
	{ return Super::ToString(Indentation) + FString::Printf(TEXT("Received at least %.2f relative damage"), NormalizedDamageThreshold); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float NormalizedDamageThreshold = 0.15f;
};

USTRUCT(DisplayName="Activation | Has tags")
struct FBehaviorEvaluatorOperationCondition_HasTags : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterSTM) const override;
	
	virtual FString ToString(int Indentation) const override
	{ return Super::ToString(Indentation) + FString::Printf(TEXT("Matches tags query [%s]"), *FilterDescription); }
	
	// just for editor
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FilterDescription;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ActorFilter;
};

USTRUCT(DisplayName="Activation | Is primary target")
struct FBehaviorEvaluatorOperationCondition_IsPrimaryTarget : public FBehaviorEvaluatorOperationCondition_Base
{
	GENERATED_BODY()
	
public:
	virtual bool Evaluate(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterSTM) const override;
	virtual FString ToString(int Indentation) const override;
	
	// if true - check if tested actor is primary target for owner in specified behavior
	// if false - change the roles of subjects: check if OWNER is primary target FOR tested actor  
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bTargetForOwner = true;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ForBehavior;	
};