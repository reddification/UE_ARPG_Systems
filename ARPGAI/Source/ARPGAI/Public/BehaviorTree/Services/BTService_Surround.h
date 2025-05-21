#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BTService_Surround.generated.h"

UCLASS(Category="Combat")
class ARPGAI_API UBTService_Surround : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTServiceSurroundMemory
	{
		TWeakObjectPtr<const class UEnemiesCoordinatorComponent> Coordinator;
	};
	
public:
	UBTService_Surround();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	FBlackboardKeySelector SelectedMobSquadRoleBBKey;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	FBlackboardKeySelector ClosestTargetBBKey;
	
};
