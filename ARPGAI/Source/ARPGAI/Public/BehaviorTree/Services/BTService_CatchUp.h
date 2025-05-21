// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CatchUp.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_CatchUp : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTMemory_CatchUp : public FBTAuxiliaryMemory
	{
		float NextDrawAttentionAt = 0.f;
	};
	
public:
	UBTService_CatchUp();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_CatchUp); };
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetBBKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector DistanceToTargetBBKey;

	UPROPERTY(EditAnywhere)
	bool bDrawAttention = true;

	UPROPERTY(EditAnywhere)
	bool bUpdateSpeed = false;

	UPROPERTY(EditAnywhere, meta=(EditCondition="bUpdateSpeed"))
	float RelativeSpeedScale = 1.5f;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="bDrawAttention"), Category="Draw attention")
	FGameplayTag DrawAttentionGestureId;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="bDrawAttention"), Category="Draw attention")
	FGameplayTag DrawAttentionPhraseId;

	UPROPERTY(EditAnywhere, meta=(EditCondition="bDrawAttention"), Category="Draw attention")
	float PhraseHeardAtRange = 1500.f;

	UPROPERTY(EditAnywhere, meta=(EditCondition="bDrawAttention"), Category="Draw attention")
	float MaxRangeToDrawAttention = 2000.f;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="bDrawAttention"), Category="Draw attention")
	float DrawAttentionCooldown = 10.f;

	UPROPERTY(EditAnywhere, meta=(EditCondition="bDrawAttention"), Category="Draw attention")
	float Loudness = 1.5f;
};
