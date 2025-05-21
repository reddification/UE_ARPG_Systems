#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_NpcState.generated.h"

UCLASS(Category = "Activities")
class ARPGAI_API UBTDecorator_NpcState : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_NpcState();
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual FString GetStaticDescription() const override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> SetByCallerParams;
};
