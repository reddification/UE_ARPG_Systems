#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_GrantTagsOnCompletion.generated.h"

USTRUCT(BlueprintType)
struct FOnBehaviorDeactivationGrantedTags
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer Tags;
	
	// GTH: Game Time Hours
	UPROPERTY(EditAnywhere)
	TOptional<float> ForDurationGTH;
};

UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_GrantTagsOnCompletion : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_GrantTagsOnCompletion();
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	UPROPERTY(EditAnywhere)
	FOnBehaviorDeactivationGrantedTags AlwaysGranted;
	
	UPROPERTY(EditAnywhere)
	TMap<TEnumAsByte<EBTNodeResult::Type>, FOnBehaviorDeactivationGrantedTags> ConditionallyGrantedTags;
};
