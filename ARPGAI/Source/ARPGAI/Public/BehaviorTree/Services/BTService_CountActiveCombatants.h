// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTService_CountActiveCombatants.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_CountActiveCombatants : public UBTService
{
	GENERATED_BODY()
	
	struct FCountAliveCharactersResult
	{
		int AliveAlliesCount = 0;
		int AliveEnemiesCount = 0;
		int DeadAlliesCount = 0;
		int DeadEnemiesCount = 0;
	};
	
public:
	UBTService_CountActiveCombatants();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	FValueOrBBKey_Float AliveAllyRelevantDistance = 1000.f;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	FValueOrBBKey_Float DeadAllyRelevantDistance = 2000.f;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	FValueOrBBKey_Float AliveEnemyRelevantDistance = 5000.f;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	FValueOrBBKey_Float DeadEnemyRelevantDistance = 1200.f;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutAliveAlliesCountBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutDeadAlliesCountBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutAliveEnemiesCountBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutDeadEnemiesCountBBKey;
	
	UPROPERTY(EditAnywhere)
	bool bUsePerception = false;
	
private:
	FCountAliveCharactersResult CountCharacters_Perception(const UBehaviorTreeComponent& OwnerComp, const UBlackboardComponent* Blackboard);
	FCountAliveCharactersResult CountCharacters_Knowledge(const UBehaviorTreeComponent& OwnerComp, const UBlackboardComponent* Blackboard);
};
