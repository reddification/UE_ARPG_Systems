// 


#include "FlowGraph/Addons/FlowNodeAddon_ActivityEQS.h"

#include "EnvironmentQuery/EnvQuery.h"

FEQSParametrizedQueryExecutionRequest* UFlowNodeAddon_ActivityEQS::GetEQSRequest(const FGameplayTag& Tag)
{
	return EqsRequests.Contains(Tag) ? &EqsRequests[Tag].EQSRequest : nullptr;
}

#if WITH_EDITOR

void UFlowNodeAddon_ActivityEQS::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.MemberProperty &&
		PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UFlowNodeAddon_ActivityEQS, EqsRequests))
	{
		for (auto& EqsRequestKVP : EqsRequests)
		{
			auto& EQSRequest = EqsRequestKVP.Value.EQSRequest;
			if (EQSRequest.QueryTemplate == nullptr)
			{
				EQSRequest.QueryConfig.Reset();
			}
			else
			{
				EQSRequest.QueryTemplate->CollectQueryParams(*this, EQSRequest.QueryConfig);
				for (const auto& CustomParamName : EqsRequestKVP.Value.CustomEqsParamsNames)
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
			
			// EqsRequest.Value.EQSRequest.PostEditChangeProperty(*this, PropertyChangedEvent);
		}
	}
}

#endif