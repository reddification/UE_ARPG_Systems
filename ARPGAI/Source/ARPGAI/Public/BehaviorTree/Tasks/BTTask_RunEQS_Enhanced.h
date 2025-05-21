

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BehaviorTree/Tasks/BTTask_RunEQSQuery.h"
#include "BTTask_RunEQS_Enhanced.generated.h"

UCLASS()
class ARPGAI_API UBTTask_RunEQS_Enhanced : public UBTTask_RunEQSQuery
{
	GENERATED_BODY()

	UBTTask_RunEQS_Enhanced(const FObjectInitializer& ObjectInitializer);
	virtual FString GetStaticDescription() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	UPROPERTY(EditAnywhere)
	TSet<FName> CustomEqsParamsNames;
};
