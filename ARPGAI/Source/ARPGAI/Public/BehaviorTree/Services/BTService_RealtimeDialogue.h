// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_RealtimeDialogue.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_RealtimeDialogue : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_RealtimeDialogue();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
