// 

#pragma once

#include "CoreMinimal.h"
#include "BTService_BehaviorEvaluator_Base.h"
#include "BTService_BehaviorEvaluator_Investigate.generated.h"

/**
 * 
 */
UCLASS(meta=(Category="Behavior evaluators"))
class ARPGAI_API UBTService_BehaviorEvaluator_Investigate : public UBTService_BehaviorEvaluator_Base
{
	GENERATED_BODY()

private:
	struct FInvestigationCandidate
	{
		FVector Location = FAISystem::InvalidLocation;
		float Score = 0.f;

		bool operator < (const FInvestigationCandidate& Other) const
		{
			return Score > Other.Score;
		}
	};
	
public:
	UBTService_BehaviorEvaluator_Investigate();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector InvestigateLocationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AttractingSoundToDistanceDependencyCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer AttractingSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ByAllyModifier = 1.5f; 	

private:
	float UpdatePerception(UBehaviorTreeComponent& OwnerComp, const FBTMemory_BehaviorEvaluator_Base* BTMemory) const;
};
