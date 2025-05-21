// 

#include "Components/PlayerBlockAssistingComponent.h"

#include "Data/CombatGameplayTags.h"
#include "Interfaces/ICombatant.h"

void UPlayerBlockAssistingComponent::StartBlocking()
{
	Super::StartBlocking();
	auto OwnerLocal = GetOwner();
	TScriptInterface<ICombatant> BlockTarget;
	auto BlockTargetActor = OwnerCombatant->GetTarget();
	if (BlockTargetActor)
	{
		if (auto BlockTargetActorCombatantInterface = Cast<ICombatant>(BlockTargetActor))
		{
			BlockTarget.SetObject(BlockTargetActor);
			BlockTarget.SetInterface(BlockTargetActorCombatantInterface);
		}
	}
	
	if (BlockTarget.GetInterface() == nullptr)
		BlockTarget = FindAutoBlockTarget(OwnerLocal);

	if (BlockTarget.GetObject() != nullptr)
		BlockTargetActor = Cast<AActor>(BlockTarget.GetObject());
	
	EMeleeAttackType ThreatAttack = EMeleeAttackType::None;
	if (BlockTarget.GetInterface() != nullptr
		&& (BlockTargetActor->GetActorLocation() - OwnerLocal->GetActorLocation()).SizeSquared() < AutoTargetSearchRange * AutoTargetSearchRange)
	{
		ThreatAttack = BlockTarget->GetActiveAttackTrajectory();
	}
	
	if (ThreatAttack == EMeleeAttackType::None)
	{
		// if no target = just do random block
		BlockDirection = OwnerCombatant->IsUsingShield()
			? FVector2D(0, 0)
			: FVector2D(FMath::RandRange(-1, 1), FMath::RandRange(-1, 1));
	}
	else
	{
		// FVector BlockDirection3D = GetIncomingAttackDirection(BlockTargetActor, ThreatAttack);
		// FVector Projection = FVector::VectorPlaneProject(BlockDirection3D, BlockTargetActor->GetActorForwardVector());
		// BlockDirection.X = Projection.Y;
		// BlockDirection.Y = Projection.Z;
		BlockDirection = GetDesiredBlockVector(ThreatAttack);
	}
}

FVector2D UPlayerBlockAssistingComponent::GetBlockInput(float DeltaTime) const
{
	return BlockDirection * BlockInputAccumulationScale * DeltaTime;
}

TScriptInterface<ICombatant> UPlayerBlockAssistingComponent::FindAutoBlockTarget(AActor* OwnerLocal)
{
	TScriptInterface<ICombatant> Result;
	TArray<FOverlapResult> Overlaps;
	// FVector Location = OwnerLocal->GetActorLocation() + OwnerLocal->GetActorForwardVector() * AutoTargetSearchRange * 0.5f;
	FVector Location = OwnerLocal->GetActorLocation();
	// auto PlayerCombatant = Cast<IPlayerCombatant>(GetOwner());
	FCollisionShape OverlapShape = FCollisionShape::MakeBox(FVector(AutoTargetSearchRange, AutoTargetSearchRange, 300.f));
	FQuat Rotation = FQuat::MakeFromRotator(OwnerLocal->GetActorRotation());
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	FCollisionObjectQueryParams CollisionObjectQueryParams;

	// @AK: I think judging simply by dot product isn't the best way.
	// Ideally best target should be judged by the shortest time until attack connects with player
	// Like protect from the one who's gonna hit you first. But currently there's no easy way of finding it out
	float BestDotProduct = -FLT_MAX;
	bool bOverlapped = GetWorld()->OverlapMultiByObjectType(Overlaps, Location, Rotation, CollisionObjectQueryParams, OverlapShape, CollisionQueryParams);
	if (!bOverlapped)
		return Result;
		
	FVector OwnerFV = OwnerLocal->GetActorForwardVector();
	for (const auto& OverlapResult : Overlaps)
	{
		auto OverlapActor = OverlapResult.GetActor();
		if (!OverlapActor)
			continue;

		auto OverlapCombatant = Cast<ICombatant>(OverlapActor);
		if (!OverlapCombatant)
			continue;

		EMeleeAttackType ActiveAttack = OverlapCombatant->GetActiveAttack();
		if (ActiveAttack == EMeleeAttackType::None)
			continue;

		const float DotProduct = (OverlapActor->GetActorLocation() - Location).GetSafeNormal() | OwnerFV;
		if (DotProduct > BestDotProduct)
		{
			Result.SetObject(OverlapActor);
			Result.SetInterface(OverlapCombatant);
		}
	}

	return Result;
}
