// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "Operations/BehaviorEvaluatorOperations_DataTypes.h"
#include "BehaviorEvaluator_Anxiety.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Anxiety")
class ARPGAI_API UBehaviorEvaluatorConfig_Anxiety : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceToAllyCalmnessScoreDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceToEnemyAnxietyScoreDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve StrengthAdvantageScoreDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve ProtectionAdvantageScoreDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGameplayTagFilterScalarValue> AllyTagsScoresScales;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGameplayTagFilterScalarValue> EnemiesTagsScoresScales;
	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Anxiety : public FBehaviorEvaluator_Base
{
private:
	using Super = FBehaviorEvaluator_Base;
	
public:
	FBehaviorEvaluator_Anxiety(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	virtual void Update(const float DeltaTime) override;
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Anxiety> AnxietyConfig;
};