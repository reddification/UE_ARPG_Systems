#pragma once
#include "BehaviorEvaluatorOperations_Base.h"
#include "BehaviorEvaluatorOperations_DataTypes.h"
#include "Data/NpcMemoryDataTypes.h"
#include "StructUtils/InstancedStruct.h"

#include "BehaviorEvaluatorOperations_Relative.generated.h"

struct FBehaviorEvaluatorOperationCondition_Base;
struct FCharacterShortTermMemory;

USTRUCT(BlueprintType)
struct ARPGAI_API FBehaviorEvaluatorOperation_Relative_Base : public FBehaviorEvaluatorOperation_Base
{
	GENERATED_BODY()
	
public:
	virtual float Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterShortTermMemory& CharacterPerceptionData, float CurrentScore) const;
	virtual FString GetShortDescription() const override;
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context,
		const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const { return FallbackValue; };	
	
	// Only evaluate if perception item detection source matches this mask.
	// If mask is empty = no filter
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mask")
	TArray<EDetectionSource> DetectionSourceMask;
	
	// if true - all detections sources must be present on perception item
	// if false - at least one
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mask")
	bool bStrictMask = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mask")
	bool bMaskCheckInverted = false;
	
private:
	bool MaskPasses(EDetectionSource DetectionSource) const;
	FString GetMaskDescription() const;

	// false is used in editor when need to update description
	EDetectionSource GetReconstructedMask() const;
};

USTRUCT(BlueprintType, DisplayName="Scalar | Const")
struct FBehaviorEvaluatorOperation_Const : public FBehaviorEvaluatorOperation_Relative_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Relative_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
		{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("%.2f"), ConstantValue); };
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override
		{ return ConstantValue; };
	virtual FString GetShortDescriptionInternal() const override 
		{ return FString::Printf(TEXT("Const %.2f"), ConstantValue) ; };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ConstantValue = 1.f;
};

USTRUCT(BlueprintType, DisplayName="Scalar | Cached Variable")
struct FBehaviorEvaluatorOperation_CachedVariable : public FBehaviorEvaluatorOperation_Relative_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Relative_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("GET [%s] | %.2f"), *CachedVariable.ToString(), FallbackValue); };
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CachedVariable = FName("variable1");
	
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override
	{ return Context.CachedVariables.Contains(CachedVariable) ? Context.CachedVariables[CachedVariable] : FallbackValue; };
	
	virtual FString GetShortDescriptionInternal() const override 
	{ return FString::Printf(TEXT("GET [%s] | %.2f"), *CachedVariable.ToString(), FallbackValue); };
};

USTRUCT(BlueprintType, DisplayName="Logic | Aggregation")
struct FBehaviorEvaluatorOperation_Aggregation : public FBehaviorEvaluatorOperation_Relative_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Relative_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;	
	virtual FString GetShortDescriptionInternal() const override { return FString::Printf(TEXT("Operations aggregations [%d]"), Operations.Num()) ; };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FBehaviorEvaluatorOperation_Relative_Base>> Operations;
};

USTRUCT(BlueprintType, DisplayName="Logic | If-Else")
struct FBehaviorEvaluatorOperation_IfElse : public FBehaviorEvaluatorOperation_Relative_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Relative_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target,
		const FCharacterShortTermMemory& CharacterPerceptionData) const override;	
	virtual FString GetShortDescriptionInternal() const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FBehaviorEvaluatorOperationCondition_Base> Condition;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FBehaviorEvaluatorOperation_Relative_Base> IfTrue;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TInstancedStruct<FBehaviorEvaluatorOperation_Relative_Base> IfFalse;
};

USTRUCT(BlueprintType, meta=(Hidden))
struct FBehaviorEvaluatorOperation_NonLinearBase : public FBehaviorEvaluatorOperation_Relative_Base
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorOperation_Relative_Base;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DependencyCurve;
};

USTRUCT(BlueprintType, DisplayName="NLD | Distance")
struct FBehaviorEvaluatorOperation_Distance : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("Distance")); };
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Distance"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Time seen")
struct FBehaviorEvaluatorOperation_TimeSeen : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("Time seen")); };
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Time seen"); };
};

// Non zero only for active behaviors
USTRUCT(BlueprintType, DisplayName="NLD | Behavior Duration")
struct FBehaviorEvaluatorOperation_BehaviorDuration : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("Behavior duration")); };
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Behavior duration"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Accumulated damage (normalized)")
struct FBehaviorEvaluatorOperation_IndividualAccumulatedDamage : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("Accumulated damage %s"), bUseLongTermMemory ? TEXT("long term") : TEXT("short term")); }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseLongTermMemory = false;
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;	
	virtual FString GetShortDescriptionInternal() const override { return FString::Printf(TEXT("Individual Accumulated Damage %s"), bUseLongTermMemory ? TEXT("long term") : TEXT("short term")); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Combat performance")
struct FBehaviorEvaluatorOperation_CombatPerformance : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("Combat Performance")); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;	
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Combat performance"); };
};

USTRUCT(BlueprintType, meta=(Hidden))
struct FBehaviorEvaluatorOperation_AdvantageBase : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
protected:
	float EvaluateAdvantage(float Numerator, float Denumerator) const;
	
	// if false - use MY damage / Target damage as curve input
	// if true - use Target damage / MY damage as curve input
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bInverted = false;
};

USTRUCT(BlueprintType, DisplayName="NLD | Advantage | Damage output advantage")
struct FBehaviorEvaluatorOperation_DamageOutputAdvantage : public FBehaviorEvaluatorOperation_AdvantageBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_AdvantageBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Damage Output Advantage"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Advantage | Protection advantage")
struct FBehaviorEvaluatorOperation_ProtectionAdvantage : public FBehaviorEvaluatorOperation_AdvantageBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_AdvantageBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Protection Output Advantage"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Advantage | Damage over protection advantage")
struct FBehaviorEvaluatorOperation_DamageOverProtectionAdvantage : public FBehaviorEvaluatorOperation_AdvantageBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_AdvantageBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Damage over Protection Advantage"); };
	
	// if true - use MY damage / TARGET protection
	// if false - use TARGET damage / MY protection
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bMyOverTarget = true;
};

USTRUCT(BlueprintType, DisplayName="NLD | Count | Other attackers on target")
struct FBehaviorEvaluatorOperation_CountOfOtherAttackersOnTarget : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("# of attackers on target"); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Count of attackers on target"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Count | Killed allies")
struct FBehaviorEvaluatorOperation_CountOfWitnessedMurderedAllies : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("# of killed allies by target"); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Count of killed allies by target"); };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float WithinGameTimeHours = 12.f;
};

USTRUCT(BlueprintType, DisplayName="NLD | Count | Allies in combat with target")
struct FBehaviorEvaluatorOperation_CountOfAlliesInCombat : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("# of allies in combat with target"); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, 
		const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Count of allies in combat with target"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Dot product | Owner FV to [Owner -> Target]")
struct FBehaviorEvaluatorOperation_DotProduct_OwnerFV_ToTarget : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("DP: NPC FV | NPC->Target"); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Dot product: owner FV to target"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Dot product | Owner FV to Target FV")
struct FBehaviorEvaluatorOperation_DotProduct_OwnerFV_TargetFV : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("DP: NPC FV | Target FV"); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Dot product: owner FV to target FV"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Dot product | Target FV to [Target -> Owner]")
struct FBehaviorEvaluatorOperation_DotProduct_TargetFV_ToOwner : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("DP: Target FV | To Owner "); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Dot product: Target FV to owner"); };
};

USTRUCT(BlueprintType, DisplayName="Scalar | Tag based")
struct FBehaviorEvaluatorOperation_Scalar_TagBased : public FBehaviorEvaluatorOperation_Relative_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Relative_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("Scalar tags based [%.2f | %.2f]"), TagBasedParameters.Value, FallbackValue); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return FString::Printf(TEXT("Scalar tags based [%.2f | %.2f]"), TagBasedParameters.Value, FallbackValue); };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagFilterScalarValue TagBasedParameters;
};

USTRUCT(BlueprintType, DisplayName="Scalar | Ally in combat")
struct FBehaviorEvaluatorOperation_Scalar_AllyInCombat : public FBehaviorEvaluatorOperation_Const
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Const;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::Super::GenerateFormulaDescription(Indentation) + FString::Printf(TEXT("Is any ally in combat with target [%.2f | %.2f]"), ConstantValue, FallbackValue); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return FString::Printf(TEXT("Is any ally in combat with target [%.2f | %.2f]"), ConstantValue, FallbackValue); };
};

USTRUCT(BlueprintType, meta=(Hidden))
struct FBehaviorEvaluatorOperation_Activation_Base : public FBehaviorEvaluatorOperation_Const
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Const;
	
protected:
	FORCEINLINE float Activate(float Value) const { return Value < ActivationThreshold ? FallbackValue : Value; };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ActivationThreshold = 0.5f;
};

USTRUCT(BlueprintType, DisplayName = "Activation | Visual contact duration")
struct FBehaviorEvaluatorOperation_Activation_VisualContactDuration : public FBehaviorEvaluatorOperation_Activation_Base
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_Activation_Base;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override;
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Activation: visual contact duration"); };
};

USTRUCT(BlueprintType, DisplayName="NLD | Count | Target allies in proximity")
struct FBehaviorEvaluatorOperation_NonLinear_CountOfTargetAlliesInProximity : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("Count of target alive allies in proximity"); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Count of target alive allies in proximity"); };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float DistanceThreshold = 400.f;
};

USTRUCT(BlueprintType, DisplayName="NLD | Count | Target allies on way to target")
struct FBehaviorEvaluatorOperation_NonLinear_CountOfTargetAlliesOnWayToTarget : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("Count of target alive allies on way to target"); }
	
protected:
	virtual float EvaluateInternal(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData) const override;
	virtual FString GetShortDescriptionInternal() const override { return TEXT("Count of target alive allies on way to target"); };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float AngleThreshold = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float DistanceThreshold = 400.f;
};

USTRUCT(BlueprintType, DisplayName="NLD | Curve Override")
struct FBehaviorEvaluatorOperation_Clamp : public FBehaviorEvaluatorOperation_NonLinearBase
{
	GENERATED_BODY()
	
	using Super = FBehaviorEvaluatorOperation_NonLinearBase;
	
public:
	virtual FString GenerateFormulaDescription(int Indentation) const override
	{ return Super::GenerateFormulaDescription(Indentation) + TEXT("Curve override"); };
	
protected:
	virtual float Evaluate(const FRelativeOperationContext& Context, const AActor* Target, const FCharacterShortTermMemory& CharacterPerceptionData, float CurrentScore) const override;
	
	virtual FString GetShortDescriptionInternal() const override 
	{ return TEXT("Curve override"); };
};

USTRUCT(BlueprintType)
struct FBehavorEvaluatorRelativeOperationsContainer : public FBehavorEvaluatorOperationsContainerBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FBehaviorEvaluatorOperation_Relative_Base>> Operations;
	
	virtual void GenerateFormulaDescription() override;
};