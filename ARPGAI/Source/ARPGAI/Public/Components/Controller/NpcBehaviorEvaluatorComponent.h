// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_LoadBehaviorContext.h"
#include "Components/ActorComponent.h"
#include "NpcBehaviorEvaluatorComponent.generated.h"


class UBehaviorTreeComponent;
class IBehaviorEvaluator;
struct FGameplayTag;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcBehaviorEvaluatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcBehaviorEvaluatorComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void RegisterBehaviorEvaluator(const TScriptInterface<IBehaviorEvaluator>& BehaviorEvaluator, const FGameplayTag& EvaluatorId);
	void UnregisterBehaviorEvaluator(const FGameplayTag& EvaluatorId);
	void InitiateBehaviorState(const FGameplayTag& EvaluatorId);
	void FinalizeBehaviorState(const FGameplayTag& BehaviorId);
	
	void RequestEvaluatorsActive(const FGameplayTagContainer& EvaluatorTags, bool bActive);
	void RequestEvaluatorsBlocked(const FGameplayTagContainer& EvaluatorTags, bool bActive);

	void RequestEvaluatorActive(const FGameplayTag& EvaluatorTag, float Duration);
	void RequestEvaluatorBlocked(const FGameplayTag& EvaluatorTag, float Duration);
	
	void Initialize(UBehaviorTreeComponent* InBTComponent, const struct FNpcDTR* NpcDTR);

protected:
	virtual void BeginPlay() override;
	
private:
	void UpdateActiveEvaluators();
	
	UPROPERTY()
	TMap<FGameplayTag, const TScriptInterface<IBehaviorEvaluator>> BehaviorEvaluators;

	UPROPERTY()
	class UNpcBlackboardDataAsset* BlackboardKeys;
	
	TWeakObjectPtr<UBehaviorTreeComponent> BTComponent;
	TMap<FGameplayTag, int> RequestedActiveEvaluators;
	TMap<FGameplayTag, int> RequestedBlockedEvaluators;
	TMap<FGameplayTag, float> PendingUnblockEvaluators;
	TMap<FGameplayTag, float> PendingRemoveEvaluatorsActivations;
};
