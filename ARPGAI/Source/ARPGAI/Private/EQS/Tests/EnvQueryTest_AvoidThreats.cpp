#include "EQS/Tests/EnvQueryTest_AvoidThreats.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/NpcCombatLogicComponent.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "GameFramework/Character.h"

UEnvQueryTest_AvoidThreats::UEnvQueryTest_AvoidThreats()
{
	Cost = EEnvTestCost::Medium;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	TestPurpose = EEnvTestPurpose::Type::Score;	
}

void UEnvQueryTest_AvoidThreats::RunTest(FEnvQueryInstance& QueryInstance) const
{
	ACharacter* QueryOwner = Cast<ACharacter>(QueryInstance.Owner.Get());
	if (QueryOwner == nullptr)
		return;

	UNpcCombatLogicComponent* MobComponent = GetNpcCombatLogicComponent(QueryOwner);
	if (MobComponent == nullptr)
		return;

	const FNpcCurrentCombatThreatsContainer& ActiveThreats = MobComponent->GetActiveThreats();
	if (ActiveThreats.Num() == 0)
		return;

	ThreatSpeedThresholdValue.BindData(QueryOwner, QueryInstance.QueryID);
	ThreatPredictionTimeValue.BindData(QueryOwner, QueryInstance.QueryID);
	GeneratorRadiusValue.BindData(QueryOwner, QueryInstance.QueryID);

	const float ThreatSpeedThreshold = ThreatSpeedThresholdValue.GetValue();
	const float ThreatPredictionTime = ThreatPredictionTimeValue.GetValue();
	const float GeneratorRadius = GeneratorRadiusValue.GetValue();
	
	TArray<FAvoidThreatData, TInlineAllocator<4>> ThreatsToAvoid;
	for (const auto& ActiveThreat : ActiveThreats)
	{
		if (ActiveThreat.Key.IsValid() == false)
			continue;
		
		const FVector& ThreatLocation = ActiveThreat.Key->GetActorLocation();
		ThreatsToAvoid.Add(FAvoidThreatData { ThreatLocation, ActiveThreat.Key.Get(), ActiveThreat.Value.BehaviorEvaluatorScore });
		FVector ThreatVelocity = ActiveThreat.Key->GetVelocity();
		if (ThreatVelocity.Size() > ThreatSpeedThreshold)
		{
			ThreatsToAvoid.Add(FAvoidThreatData { ThreatLocation + ThreatVelocity * ThreatPredictionTime, 
				ActiveThreat.Key.Get(), ActiveThreat.Value.BehaviorEvaluatorScore });
		}	
	}
	
	// first, trace from each it point to all active threats approximate locations
	// if tracing, score += threat score / Distance
	// some normalization might be required
	float VerticalTraceOffset = QueryOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.8f;
	FCollisionQueryParams CollisionQueryParams;
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		float Score = 0.f;
		FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		float ItemToMobDistance = FVector::Dist(ItemLocation, QueryOwner->GetActorLocation());
		for (const FAvoidThreatData& ThreatToAvoid : ThreatsToAvoid)
		{
			CollisionQueryParams.ClearIgnoredSourceObjects();
			CollisionQueryParams.AddIgnoredActor(QueryOwner);
			CollisionQueryParams.AddIgnoredActor(ThreatToAvoid.Actor);
			FHitResult HitResult;
			TArray<FVector, TInlineAllocator<3>> ItemLocations;
			ItemLocations.Emplace(ItemLocation);
			ItemLocations.Emplace(ItemLocation + FVector::UpVector * VerticalTraceOffset);
			ItemLocations.Emplace(ItemLocation - FVector::UpVector * VerticalTraceOffset);

			bool bInCover = true;
			for (int i = 0; i < 3; i++)
			{
				bInCover = GetWorld()->LineTraceSingleByChannel(HitResult, ThreatToAvoid.Location + FVector::UpVector * 60.f, ItemLocations[i],
				ECC_Visibility, CollisionQueryParams);
				if (bInCover == false)
					break;
			}
			
			if (bInCover)
			{
				// Best locations are the closest 
				float ClampedItemToMobDistance = FMath::Max(1.f, ItemToMobDistance);
				Score += ThreatToAvoid.Score / ClampedItemToMobDistance * GeneratorRadius;
			}
		}

		It.SetScore(TestPurpose, FilterType, Score, -FLT_MAX, FLT_MAX);
	}
}

FText UEnvQueryTest_AvoidThreats::GetDescriptionDetails() const
{
	FString Description = FString::Printf(TEXT("Calculate score how good this point is to cover from threats from UNpcCombatLogicComponent"));
	Description = Description.Append(FString::Printf(TEXT("\nThreat location prediction time: %.2fs\nThreat min speed threshold = %.2f"),
		ThreatPredictionTimeValue.GetValue(), ThreatSpeedThresholdValue.GetValue()));
	return FText::FromString(Description);
}
