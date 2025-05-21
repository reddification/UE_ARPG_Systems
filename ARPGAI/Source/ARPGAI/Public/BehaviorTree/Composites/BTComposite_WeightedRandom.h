

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BTComposite_WeightedRandom.generated.h"

UCLASS()
class ARPGAI_API UBTComposite_WeightedRandom : public UBTCompositeNode
{
	GENERATED_BODY()

public:
	UBTComposite_WeightedRandom();
	virtual int32 GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const override;

protected:
	// @AK 09.12.2024 fucking UE 5.4 gone insane and gives some obscure errors which are resolved by overriding some virtual methods with just Super:: invocation
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
};
