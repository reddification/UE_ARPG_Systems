

#include "BehaviorTree/Tasks/BTTask_RunEQS_Enhanced.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "VisualLogger/VisualLogger.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_RunEQS_Enhanced::UBTTask_RunEQS_Enhanced(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	NodeName = "Run EQS Query (enhanced)";
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RunEQS_Enhanced, BlackboardKey));
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RunEQS_Enhanced, BlackboardKey), AActor::StaticClass());
}

FString UBTTask_RunEQS_Enhanced::GetStaticDescription() const
{
	FString Result = Super::GetStaticDescription();

	for (const auto& QueryConfigItem : EQSRequest.QueryConfig)
	{
		if (QueryConfigItem.BBKey.IsSet())
		{
			Result = Result.Append(FString::Printf(TEXT("\n%s = %s"), *QueryConfigItem.ParamName.ToString(), *QueryConfigItem.BBKey.SelectedKeyName.ToString()));
		}
		else
		{
			Result = Result.Append(FString::Printf(TEXT("\n%s = %.2f"), *QueryConfigItem.ParamName.ToString(), QueryConfigItem.Value));
		}
	}

	Result = Result.Append(FString::Printf(TEXT("\nRun Mode: %s"), *UEnum::GetDisplayValueAsText<EEnvQueryRunMode::Type>(EQSRequest.RunMode).ToString()));
	return Result;
}

#if WITH_EDITOR

void UBTTask_RunEQS_Enhanced::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty &&
		PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UBTTask_RunEQSQuery, EQSRequest))
	{
		for (const auto& CustomParamName : CustomEqsParamsNames)
		{
			if (EQSRequest.QueryConfig.ContainsByPredicate([CustomParamName] (const FAIDynamicParam& DynamicParam){ return DynamicParam.ParamName == CustomParamName; }))
				continue;
			
			FAIDynamicParam CustomParam = FAIDynamicParam();
			CustomParam.ParamName = CustomParamName;
			CustomParam.ParamType = EAIParamType::Float; // only float for now
			CustomParam.Value = 0.f;
			CustomParam.ConfigureBBKey(*this);

			EQSRequest.QueryConfig.Add(CustomParam);
		}
	}
}

#endif
