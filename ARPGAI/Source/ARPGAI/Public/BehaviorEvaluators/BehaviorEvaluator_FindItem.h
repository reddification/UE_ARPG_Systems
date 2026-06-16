#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "BehaviorEvaluator_FindItem.generated.h"

UCLASS(DisplayName="Find item")
class ARPGAI_API UBehaviorEvaluatorConfig_FindItem : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_FindItem();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBlackboardKeySelector FoundActorBBKey;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer ItemsIdsFilter;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve ItemScoreDistanceDependency;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ItemNotFoundEventTag;

	// this value is subtracted so keep it positive
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float UtilityReductionOnSearchFailed = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bConsiderHostileActors = false;

	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_FindItem : public FBehaviorEvaluator_Base
{
	using Super = FBehaviorEvaluator_Base;

public:
	FBehaviorEvaluator_FindItem(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	
	virtual void Update(const float DeltaTime) override;
	
protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;
	virtual void HandleMessage_Internal(const FGameplayTag& MessageTag) override;
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_FindItem> FindItemConfig;
	TWeakObjectPtr<AActor> CachedFoundItem;
};