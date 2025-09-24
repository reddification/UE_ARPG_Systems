// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardAssetProvider.h"
#include "BehaviorTree/BlackboardData.h"
#include "UObject/Object.h"
#include "BehaviorEvaluatorBase.generated.h"

class UNpcAttitudesComponent;
class AAIController;
/**
 * Think of these as modular/plug-in-play stateful BT services: NPC can turn them on and off on demand depending on its current behavior
 * For example, a beast can have a BehaviorEvaluator_Prey where it will search for smaller beasts to feed on by scanning AIPerceptionComponent sight perception
 * But BehaviorEvaluator_Prey is irrelevant when, say, it is night and the beast is running "sleep" behavior
 * Or say there's BehaviorEvaluator_UnexpectedDamage, which is relevant when beast has no active enemy, but pointless, when NPC already has got an enemy
 * Activation control is implemented by putting BTDecorator_AddBehaviorEvaluator in BTs where it makes sense
 * (e.g. activate "BehaviorEvaluator_EnemyAwareness" in both Idle behavior and combat behavior)
 * The behavior evaluators should be mapped by gameplay tags in NpcDTR
 * Since behavior evaluators are stateful and parametrized, suggested usage is: expose them as instanced object in data table/data asset
 * but then make individual copies to each NPC on initialization
 */
UCLASS()
class ARPGAI_API UBehaviorEvaluatorBase : public UObject, public IBlackboardAssetProvider, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UBehaviorEvaluatorBase();
	virtual void Activate(AAIController* InAIController);
	virtual void Deactivate();
	
	FORCEINLINE bool IsActive() const { return bActive; } 

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UBlackboardData> BlackboardAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bTick = true;
	
	// 0 - update every tick
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, EditCondition=bTick))
	float TickInterval = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector UtilityBBKey;

	TWeakObjectPtr<AAIController> AIController;
	TWeakObjectPtr<UBlackboardComponent> BlackboardComponent;
	TWeakObjectPtr<UNpcAttitudesComponent> NpcAttitudesComponent;

	void ChangeUtility(const float DeltaUtility);
	virtual bool IsHostile(AActor* Actor);
	virtual void Evaluate();

public: // IBlackboardAssetProvider
	virtual UBlackboardData* GetBlackboardAsset() const override;

public: //FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override final { return bTick && bActive; };
	virtual TStatId GetStatId() const override;
	
private:
	bool bActive = false;
	float TimeSinceLastUpdate = 0.f;
};
