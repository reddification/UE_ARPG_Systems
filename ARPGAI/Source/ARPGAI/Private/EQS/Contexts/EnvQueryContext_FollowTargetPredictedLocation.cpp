// 


#include "EQS/Contexts/EnvQueryContext_FollowTargetPredictedLocation.h"

#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"
#include "DataProviders/AIDataProvider_QueryParams.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

UEnvQueryContext_FollowTargetPredictedLocation::UEnvQueryContext_FollowTargetPredictedLocation()
{
	// auto DataBinding = NewObject<UAIDataProvider_QueryParams>();
	// DataBinding->ParamName = FName("PredictionTime");
	// DataBinding->FloatValue = 0.f;
	// PredictionTimeParam.DataBinding = DataBinding;
}

void UEnvQueryContext_FollowTargetPredictedLocation::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                   FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn) || !QuerierPawn->Controller)
	{
		return;
	}

	auto NpcComponent = QuerierPawn->FindComponentByClass<UNpcComponent>();
	if (!NpcComponent)
		return;

	auto FollowTarget = NpcComponent->GetFollowTarget();
	if (FollowTarget == nullptr)
		return;

	// TODO make it a native gameplay tag 
	const float* PredictionTimePtr = QueryInstance.NamedParams.Find(FName("PredictionTime"));
	PredictionTimeParam.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
	const float PredictionTimeTest = PredictionTimeParam.GetValue();
	const float PredictionTime = PredictionTimePtr != nullptr ? *PredictionTimePtr : 0.f;
	FVector ExpectedLocation = FollowTarget->GetActorLocation() + FollowTarget->GetVelocity() * PredictionTime;
	UEnvQueryItemType_Point::SetContextHelper(ContextData, ExpectedLocation);

	UE_VLOG_CAPSULE(QuerierPawn, LogARPGAI, VeryVerbose, ExpectedLocation - FVector::UpVector * 90.f, 90.f, 30.f,
		FQuat::Identity, FColor::White, TEXT("Follow target predicted location"));
}
