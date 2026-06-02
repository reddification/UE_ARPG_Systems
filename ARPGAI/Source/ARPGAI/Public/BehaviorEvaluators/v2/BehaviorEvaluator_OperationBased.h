#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluators/BehaviorEvaluator_Base.h"
#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Aggregating.h"
#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Relative.h"
#include "BehaviorEvaluator_OperationBased.generated.h"

bool IsDetectable(EDetectionSource DetectionSource);

/**
 * 
 */
USTRUCT(BlueprintType)
struct FEntityOperationEvaluationParameters
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery EntityRelevantQuery;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBehavorEvaluatorRelativeOperationsContainer IndividualOperationsContainer;
	
	// prevents NPCs from chasing packs of rats (or whatever low-value enemies)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve EntityOrderedScoreScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve EntityPressureSaturationCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString DebugInfo = "";
};

UCLASS(meta=(Hidden))
class ARPGAI_API UBehaviorEvaluatorConfig_OperationBased : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()

public:
	virtual void GenerateFormulasDescriptions();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBehavorEvaluatorAggregatingOperationsContainer StatePressureOperationsContainer;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve StatePressureSaturationCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FEntityOperationEvaluationParameters EnemiesEvaluationParameters;
};

class FBehaviorEvaluator_OperationBased : public FBehaviorEvaluator_Base
{
private:
	using Super = FBehaviorEvaluator_Base;
	
protected:
	
	struct FBehaviorEvaluator_ActorScore
	{
		AActor* Actor = nullptr;
		float Score = 0.f;
		EDetectionSource DetectionSource = EDetectionSource::None;
		bool bAlive = true;
		
		bool operator < (const FBehaviorEvaluator_ActorScore& Other) const
		{
			const float ThisKnowsLocationScale = IsDetectable(DetectionSource) ? 10.f : 0.f;
			const float OtherKnowsLocationScale = IsDetectable(Other.DetectionSource) ? 10.f : 0.f;
			const float ThisAliveScale = bAlive ? 1.f : 0.1f;
			const float OtherAliveScale = Other.bAlive ? 1.f : 0.1f;
			return Score * ThisKnowsLocationScale * ThisAliveScale > Other.Score * OtherKnowsLocationScale * OtherAliveScale;
		}
	};
	
	using FActorScoresContainer = TArray<FBehaviorEvaluator_ActorScore, TInlineAllocator<8>>;
	using FOperationsContainer = TArray<TInstancedStruct<FBehaviorEvaluatorOperation_Relative_Base>>;
	
public:
	FBehaviorEvaluator_OperationBased(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);

protected:
	virtual void Update(const float DeltaTime) override;
	FAggregationOperationContext GetAggregationOperationContext() const;
	FRelativeOperationContext GetRelativeOperationContext() const;
	virtual float Evaluate();
	float CalculateStatePressure() const;
	
	virtual bool IsCharacterRelevant(const FCharacterPerceptionData& CharacterPerceptionData, 
		const FEntityOperationEvaluationParameters& EvaluationParameters) const;
	virtual void OnIndividualScoreCalculated(AActor* Actor, const FCharacterPerceptionData& Value, float IndividualScore) {};
	virtual void PreEvaluate() {};
	
	void ExecuteEntityOperations(AActor* Target, const FCharacterPerceptionData& CharacterPerception,
	    const FRelativeOperationContext& RelativeOperationData, FActorScoresContainer& Container,
	    const FEntityOperationEvaluationParameters& EvaluationParameters);
	float GetEntitiesAggregatedScore(FActorScoresContainer& ActorScores, const FEntityOperationEvaluationParameters& EvaluationParameters) const;
	
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_OperationBased> OperationsConfig;
	
private:
	float Health = 0.f;
	float MaxHealth = 0.f;
};
