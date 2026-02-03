

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateCombatRole.generated.h"

class UNpcCombatLogicComponent;
class UEnemiesCoordinatorComponent;

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_UpdateCombatRole : public UBTService
{
	GENERATED_BODY()
	
private:
	struct FBTMemory_UpdateCombatRole
	{
		TWeakObjectPtr<UEnemiesCoordinatorComponent> CoordinatorComponent;
		TWeakObjectPtr<UNpcCombatLogicComponent> CombatLogicComponent;
	};
	
public:
	UBTService_UpdateCombatRole();
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetActorBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutNpcCombatRoleTypeBBKey;

private:
	EBlackboardNotificationResult OnTargetChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	void TrySetCoordinatorRegistration(FBTMemory_UpdateCombatRole* BTMemory, APawn* BotCharacter,
		UBlackboardComponent* BlackboardComponent, bool bRegister) const;
};
