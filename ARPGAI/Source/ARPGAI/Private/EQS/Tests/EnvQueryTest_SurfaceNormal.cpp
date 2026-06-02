// 


#include "EQS/Tests/EnvQueryTest_SurfaceNormal.h"

#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"

UEnvQueryTest_SurfaceNormal::UEnvQueryTest_SurfaceNormal()
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	TestPurpose = EEnvTestPurpose::Type::FilterAndScore;	
	TraceDownDistanceParam.DefaultValue = 1000.f;
}

void UEnvQueryTest_SurfaceNormal::RunTest(FEnvQueryInstance& QueryInstance) const
{
	auto QueryOwner = QueryInstance.Owner.Get();
	FCollisionQueryParams CollisionQueryParams;
	if (auto Actor = Cast<AActor>(QueryOwner))
		CollisionQueryParams.AddIgnoredActor(Actor);
	else if (auto ActorComponent = Cast<UActorComponent>(QueryOwner))
		CollisionQueryParams.AddIgnoredActor(ActorComponent->GetOwner());
	
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();
	
	TraceDownDistanceParam.BindData(QueryOwner, QueryInstance.QueryID);
	float TraceDownDistance = TraceDownDistanceParam.GetValue();
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, ItemLocation, ItemLocation - FVector::UpVector * TraceDownDistance, TraceChannel, CollisionQueryParams);
		float ImpactNormalToWorldUpDP = 1.f;
		if (bHit)
			ImpactNormalToWorldUpDP = HitResult.ImpactNormal | FVector::UpVector;
		
		It.SetScore(TestPurpose, FilterType, ImpactNormalToWorldUpDP, MinThresholdValue, MaxThresholdValue);
	}
}

FText UEnvQueryTest_SurfaceNormal::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}
