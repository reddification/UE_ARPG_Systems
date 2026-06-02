#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SayPhrase.generated.h"

UCLASS(Category="Emotes")
class ARPGAI_API UBTTask_SayPhrase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SayPhrase();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FGameplayTag DetermineActualSpeechTag(UBehaviorTreeComponent& OwnerComp);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SayPhrase")
	FGameplayTagContainer PhraseOptions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SayPhrase")
	FBlackboardKeySelector PhraseTagBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bReportNoiseEvent = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bReportNoiseEvent"))
	float Loudness = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bReportNoiseEvent"))
	float Range = 1500.f;

};
