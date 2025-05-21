// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_NpcSpeak.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_NpcSpeak : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_NpcSpeak();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.Noise,AI.Speech"))
	FGameplayTag AISoundTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AISoundTagBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Loudness = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range = 1500.f;
};
