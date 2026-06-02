#include "EQS/Generators/EnvQueryGenerator_Cone_Extended.h"

void UEnvQueryGenerator_Cone_Extended::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	TArray<AActor*> CenterActors;
	
	ContextRotationOffsetValue.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
	const float ConeDegreesValue = FMath::Clamp(FMath::Abs(ConeDegrees.GetValue()), 0.f, 359.f);
	if (FMath::IsNearlyZero(ConeDegreesValue))
	{
		Super::GenerateItems(QueryInstance);
		return;
	}

	QueryInstance.PrepareContext(CenterActor, CenterActors);
	if (CenterActors.Num() <= 0)
		return;
	
	BindDataToDataProviders(QueryInstance);
	
	//Get the values from each data provider
	const float AlignedPointDistanceValue = AlignedPointsDistance.GetValue();
	const float AngleStepValue = FMath::Clamp(AngleStep.GetValue(), 1.f, 359.f);
	const float GenerationRange = FMath::Clamp(Range.GetValue(), 0.f, MAX_flt);
	const float ContextOffsetAngle = ContextRotationOffsetValue.GetValue();
	const int32 MaxPointsPerAngleValue = FMath::FloorToInt(GenerationRange / AlignedPointDistanceValue);

	TArray<FNavLocation> GeneratedItems;
	GeneratedItems.Reserve(CenterActors.Num() * FMath::CeilToInt(ConeDegreesValue / AngleStepValue) * MaxPointsPerAngleValue + 1);
	
	//Generate points for each actor
	for (int32 CenterIndex = 0; CenterIndex < CenterActors.Num(); CenterIndex++)
	{
		const FVector ForwardVector = ContextOffsetAngle != 0.f
			? CenterActors[CenterIndex]->GetActorForwardVector().RotateAngleAxis(ContextOffsetAngle, CenterActors[CenterIndex]->GetActorUpVector())
			: CenterActors[CenterIndex]->GetActorForwardVector();
		
		const FVector ActorLocation = CenterActors[CenterIndex]->GetActorLocation();
		
		for (float Angle = -(ConeDegreesValue * 0.5f); Angle < (ConeDegreesValue * 0.5f); Angle += AngleStepValue)
		{
			const FVector AxisDirection = ForwardVector.RotateAngleAxis(Angle, FVector::UpVector);

			// skipping PointIndex == 0 as that's the context's location
			for (int32 PointIndex = 1; PointIndex <= MaxPointsPerAngleValue; PointIndex++)
			{
				const FVector GeneratedLocation = ActorLocation + (AxisDirection * PointIndex * AlignedPointDistanceValue);
				GeneratedItems.Add(FNavLocation(GeneratedLocation));
			}
		}

		if (bIncludeContextLocation)
		{
			GeneratedItems.Add(FNavLocation(ActorLocation));
		}
	}	

	ProjectAndFilterNavPoints(GeneratedItems, QueryInstance);
	StoreNavPoints(GeneratedItems, QueryInstance);
}
