
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetStateEffects.generated.h"

UCLASS(HideCategories=(Service))
class ARPGAI_API UBTService_SetStateEffects : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetStateEffects();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	FGameplayTag GetActualStateTag(UBehaviorTreeComponent& OwnerComp) const;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.State"))
	FGameplayTag NewStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector NewStateTagBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="SetByCaller,AI.SetByCaller"))
	TMap<FGameplayTag, float> SetByCallerParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ActivationDelay = 0.f;
};
