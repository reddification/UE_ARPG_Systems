// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_ActivityParameters.h"
#include "AddOns/FlowNodeAddOn.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "FlowNodeAddon_ActivityEQS.generated.h"


USTRUCT(BlueprintType)
struct FActivityEQSRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSet<FName> CustomEqsParamsNames;

	UPROPERTY(EditAnywhere)
	FEQSParametrizedQueryExecutionRequest EQSRequest;
};

UCLASS()
class ARPGAI_API UFlowNodeAddon_ActivityEQS : public UFlowNodeAddon_ActivityParameters
{
	GENERATED_BODY()
	
public:
	FEQSParametrizedQueryExecutionRequest* GetEQSRequest(const FGameplayTag& Tag);
	
protected:
	UPROPERTY(Category = "EQS|Request", EditAnywhere)
	TMap<FGameplayTag, FActivityEQSRequest> EqsRequests;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
