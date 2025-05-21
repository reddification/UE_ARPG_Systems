

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_RegisterInCoordinator.generated.h"

class UEnemiesCoordinatorComponent;

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_RegisterInCoordinator : public UBTService
{
	GENERATED_BODY()
	
private:
	struct FRegisterInCoordinatorMemory
	{
		TWeakObjectPtr<UEnemiesCoordinatorComponent> Coordinator;
	};
	
public:
	UBTService_RegisterInCoordinator();
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetActorBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutInitialMobSquadRoleTypeBBKey;

private:
	void TrySetCoordinatorRegistration(UEnemiesCoordinatorComponent* CoordinatorComponent, APawn* BotCharacter,
	UBlackboardComponent* BlackboardComponent, bool bRegister) const;
};
