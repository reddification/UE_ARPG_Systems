

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_RangeAttack.generated.h"

class AMCAICharacter;
/**
 * 
 */
UCLASS(Category="Combat")
class ARPGAI_API UBTService_RangeAttack : public UBTService
{
	GENERATED_BODY()
private:
	struct FAttackNodeMemory
	{
		float AttackCooldown = 0.f;
		float AttackCooldownDeviationRatio = 0.f;
		float AttackRange = 0.0;
		float AttackTimeLeft = 0.f;
		float TakeAimTimeLeft = 0.f;
		float CurrentAttackCooldown = 0.f;
		bool IsAiming = false;

		bool IsValid() const;
	};
	
public:
	UBTService_RangeAttack();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AttackTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AimTag;
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector IsAttackOnCooldownBBKey;
};
