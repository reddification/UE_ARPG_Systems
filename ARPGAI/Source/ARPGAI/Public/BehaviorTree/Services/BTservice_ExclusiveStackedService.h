#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTservice_ExclusiveStackedService.generated.h"

/**
 * 24 Mar 2026 (aki): блять умоляю моську только не разбейте мне за такое 
 */
UCLASS()
class ARPGAI_API UBTservice_ExclusiveStackedService : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTservice_ExclusiveStackedService();
	
	void Freeze(uint8* NodeMemory);
	void Unfreeze(uint8* NodeMemory);
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	FName StackItemKey;
};
