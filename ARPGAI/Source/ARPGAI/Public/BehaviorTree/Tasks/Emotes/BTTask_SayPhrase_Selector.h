#pragma once

#include "CoreMinimal.h"
#include "BTTask_SayPhrase.h"
#include "Data/CommonWrapperTypes.h"
#include "BTTask_SayPhrase_Selector.generated.h"

UCLASS(HideCategories=("SayPhrase"), Category="Emotes")
class ARPGAI_API UBTTask_SayPhrase_Selector : public UBTTask_SayPhrase
{
	GENERATED_BODY()
	
public:
	UBTTask_SayPhrase_Selector();
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual FGameplayTag DetermineActualSpeechTag(UBehaviorTreeComponent& OwnerComp) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ScalarSelectorBBKey;

	// Keep options sorted. Option selection goes in array items order with <= comparision
	UPROPERTY(EditAnywhere)
	TArray<FTagFloatPair> Options;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag FallbackOption;
};
