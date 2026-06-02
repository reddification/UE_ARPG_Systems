#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_OperationBased.h"
#include "BehaviorEvaluators/BehaviorEvaluator_Base.h"
#include "BehaviorEvaluator_Anxiety_v2.generated.h"

/*
 * Not a behavior utility, but still is used to write anxiety score to blackboard for behavior options
 */
UCLASS(DisplayName = "Anxiety v2")
class ARPGAI_API UBehaviorEvaluatorConfig_Anxiety_v2 : public UBehaviorEvaluatorConfig_OperationBased
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FEntityOperationEvaluationParameters AlliesEvaluationParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> AnxietyInducingSounds;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AnxietySoundToDistance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve SoundDotProductToAnxietyScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AnxietyFromSoundsSaturationCurve;

	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
	
	virtual void GenerateFormulasDescriptions() override;
};

class FBehaviorEvaluator_Anxiety_v2 : public FBehaviorEvaluator_OperationBased
{
	using Super = FBehaviorEvaluator_OperationBased;

public:
	FBehaviorEvaluator_Anxiety_v2(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	virtual float Evaluate() override;

private:
	
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Anxiety_v2> AnxietyConfig;
};