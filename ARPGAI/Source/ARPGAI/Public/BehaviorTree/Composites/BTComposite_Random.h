

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "UObject/Object.h"
#include "BTComposite_Random.generated.h"

UCLASS()
class ARPGAI_API UBTComposite_Random : public UBTCompositeNode
{
	GENERATED_BODY()

public:
	UBTComposite_Random();
	virtual int32 GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const override;
	virtual FString GetStaticDescription() const override;

protected:
	// @AK 09.12.2024 fucking UE 5.4 gone insane and gives some obscure errors which are resolved by overriding some virtual methods with just Super:: invocation
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
};
