// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_GrantTagsOnActivation.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_GrantTagsOnActivation : public UBTDecorator
{
	GENERATED_BODY()
	
private:
	struct FBTMemory_GrantedTags
	{
		FGameplayTag GrantedTag;
	};
	
public:
	UBTDecorator_GrantTagsOnActivation();
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override 
	{ return bJustOneRandom && GrantedTags.Num() > 1 ? sizeof(FBTMemory_GrantedTags) : Super::GetInstanceMemorySize(); };
	
protected:
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditAnywhere)
	bool bJustOneRandom = false;	
	
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
};
