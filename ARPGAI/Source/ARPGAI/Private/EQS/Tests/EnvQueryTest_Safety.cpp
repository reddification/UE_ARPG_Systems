// Fill out your copyright notice in the Description page of Project Settings.


#include "EQS/Tests/EnvQueryTest_Safety.h"

#include "Components/CapsuleComponent.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "GameFramework/Character.h"

UEnvQueryTest_Safety::UEnvQueryTest_Safety()
{
	Cost = EEnvTestCost::Medium;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	TestPurpose = EEnvTestPurpose::Type::Score;
	SetWorkOnFloatValues(true);
}

void UEnvQueryTest_Safety::RunTest(FEnvQueryInstance& QueryInstance) const
{
	ACharacter* QueryOwner = Cast<ACharacter>(QueryInstance.Owner.Get());
	if (QueryOwner == nullptr)
		return;

	ScoringFactor.BindData(QueryOwner, QueryInstance.QueryID);
	float ScoringFactorValue = ScoringFactor.GetValue();
	if (ScoringFactorValue == 0.f)
		return;
	
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();
	
	TRACE_CPUPROFILER_EVENT_SCOPE(UEnvQueryTest_Safety::RunTest)
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(QueryOwner);
	const float CharacterEyesLocationOffset = QueryOwner->BaseEyeHeight + QueryOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		FVector ItemLocation = GetItemLocation(QueryInstance, It);
		// FCollisionObjectQueryParams CollisionObjectParams;
		// CollisionObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		// CollisionObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(QueryOwner);

		FCollisionShape Box = FCollisionShape::MakeBox(FVector(125, 125, 40));
		bool bHaveObstacleNearby = GetWorld()->OverlapBlockingTestByChannel(ItemLocation + FVector::UpVector * 100.f, FQuat::Identity, TraceChannel_ObstacleOverlap, Box, CollisionQueryParams);
#if WITH_EDITOR
		FBox LogBox = FBox::BuildAABB(ItemLocation + FVector::UpVector * 100.f, FVector(125, 125, 40) * 2);
		UE_VLOG_BOX(QueryOwner, LogARPGAI_EQS, VeryVerbose, LogBox, FColor::White, TEXT("Overlap test"));
#endif
		if (!bHaveObstacleNearby)
			continue;
		
		const FVector PrimaryTraceLocation = ItemLocation + CharacterEyesLocationOffset * FVector::UpVector;
		UE_VLOG_LOCATION(QueryOwner, LogARPGAI_EQS, Verbose, PrimaryTraceLocation, 10.f, FColor::Cyan, TEXT("Testing safety"));

		float Score = 0.f;
		constexpr float SideOffset = 150.f;
		float VerticalOffsets[2] = { 75.f, 150.f };
		// 4 May 2026 (aki): TODO use parallel for
		for (int Angle = 0; Angle < 360; Angle += 90)
		{
			for (int i = 0; i < 2; i++)
			{
				FVector ContextLocation = ItemLocation
					+ FVector::ForwardVector.RotateAngleAxis(Angle, FVector::UpVector).GetSafeNormal() * SideOffset
					+ FVector::UpVector * VerticalOffsets[i];

				UE_VLOG_LOCATION(QueryOwner, LogARPGAI_EQS, Verbose, ContextLocation, 10.f, FColor::White, TEXT("Safety probe"));

				FHitResult HitResult;
				bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, PrimaryTraceLocation, ContextLocation, TraceChannel_Probe, CollisionParams);
				if (bHit && HitResult.GetActor() && !HitResult.GetActor()->IsA<APawn>())
					Score += 1.f;
			}
		}
		
		Score /= 8.f; // divide by count of probes
		
		It.SetScore(TestPurpose, FilterType, Score, MinThresholdValue, MaxThresholdValue);
	}
}

FText UEnvQueryTest_Safety::GetDescriptionTitle() const
{
	return FText::FromString(TEXT("Safety: How good spot obstructs visibility"));
}

FText UEnvQueryTest_Safety::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}
