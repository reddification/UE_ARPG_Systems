// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "UObject/Object.h"
#include "BehaviorEvaluator_Heal.generated.h"

class INpcAliveCreature;
/**
 * 
 */
UCLASS(DisplayName="Heal")
class ARPGAI_API UBehaviorEvaluatorConfig_Heal : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve NormalizedHealthToDesireDependency;
	
	// subtracted from desire
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceToEnemiesReluctanceDependency;
	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Heal : public FBehaviorEvaluator_Base
{
	using Super = FBehaviorEvaluator_Base;
	
public:
	FBehaviorEvaluator_Heal(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	virtual void Update(const float DeltaTime) override;
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Heal> HealConfig;
};
