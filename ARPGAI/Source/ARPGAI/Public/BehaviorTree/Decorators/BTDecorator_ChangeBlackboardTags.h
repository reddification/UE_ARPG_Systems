// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_ChangeBlackboardTags.generated.h"

USTRUCT(BlueprintType)
struct FBlackboardKeyTagsOperation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Tags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAdd = true;
};

UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_ChangeBlackboardTags : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_ChangeBlackboardTags();
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TagsBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeyTagsOperation OnActivation;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeyTagsOperation OnDeactivationAlways;
	
	UPROPERTY(EditAnywhere)
	TMap<TEnumAsByte<EBTNodeResult::Type>, FBlackboardKeyTagsOperation> OnDeactivationConditional;
};
