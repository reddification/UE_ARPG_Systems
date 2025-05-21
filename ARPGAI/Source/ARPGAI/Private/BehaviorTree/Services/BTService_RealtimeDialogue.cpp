// 


#include "BehaviorTree/Services/BTService_RealtimeDialogue.h"

UBTService_RealtimeDialogue::UBTService_RealtimeDialogue()
{
	NodeName = "Maintain realtime dialogue";
}

void UBTService_RealtimeDialogue::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
}
