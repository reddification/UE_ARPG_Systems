#include "Components/TargetLockComponent.h"

#include "Data/MeleeCombatSettings.h"
#include "Engine/OverlapResult.h"
#include "Interfaces/CombatAliveCreature.h"
#include "Interfaces/LockableTarget.h"
#include "Perception/AISightTargetInterface.h"

void UTargetLockComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!ensure(Target.IsValid())) // this component shouldn't tick without a target
		return;

	FVector OwnerEyesLocation;
	FRotator OwnerViewRotation;
	auto OwnerLocal = GetOwner();
	OwnerLocal->GetActorEyesViewPoint(OwnerEyesLocation, OwnerViewRotation);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());

	auto Targetable = Cast<ITargetable>(Target.Get());
	if (!Targetable->CanTarget(GetOwner()))
	{
		ClearTarget();
		return;
	}

	FCanBeSeenFromContext CanBeSeenFromContext;
	CanBeSeenFromContext.IgnoreActor = GetOwner();
	CanBeSeenFromContext.ObserverLocation = OwnerEyesLocation;
	
	if (auto ObservableActor = Cast<IAISightTargetInterface>(Target.Get()))
	{
		FVector SeenAtLocation;
		int i = 0;
		int async_i = 0;
		float f = 0.f;

		auto CanBeSeenFromResult = ObservableActor->CanBeSeenFrom(CanBeSeenFromContext, SeenAtLocation, i, async_i, f);
		if (CanBeSeenFromResult != UAISense_Sight::EVisibilityResult::Visible)
			ClearTarget();
	}
	else
	{
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, OwnerEyesLocation, Target->GetActorLocation(), ECC_Visibility, CollisionQueryParams);
		if (!bHit || HitResult.GetActor() != Target)
			ClearTarget();
	}
}

AActor* UTargetLockComponent::FindTarget()
{
	TArray<FOverlapResult> Overlaps;
	FVector OwnerEyesLocation;
	FRotator OwnerViewRotation;
	auto OwnerLocal = GetOwner();
	OwnerLocal->GetActorEyesViewPoint(OwnerEyesLocation, OwnerViewRotation);
	// auto PlayerCombatant = Cast<IPlayerCombatant>(GetOwner());
	FVector ViewDirection = OwnerViewRotation.Vector(); //PlayerCombatant->GetPlayerCombatantViewDirection();
	FVector Location = GetOwner()->GetActorLocation() + OwnerViewRotation.Vector() * MaxDistance * 0.5f;
	FCollisionShape OverlapShape = FCollisionShape::MakeBox(FVector(MaxDistance * 0.5f, MaxDistance * 0.5f, 300.f));
	FQuat Rotation = FQuat::MakeFromRotator(OwnerViewRotation);
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	// in theory if at any point it would become necessary to target non-npc actors (like destructible items) - we should either add a new object type for them, or use WorldDynamic
	// CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	auto MeleeCombatSettings = GetDefault<UMeleeCombatSettings>();
	for (const TEnumAsByte<ECollisionChannel>& TargetLockObjectChannel : MeleeCombatSettings->TargetLockObjectChannels)
		CollisionObjectQueryParams.AddObjectTypesToQuery(TargetLockObjectChannel.GetValue());
	
	// These parameters is only to pass it to IAISightTargetInterface::CanBeSeenFrom. Required parameter
	int TracesThisTick = 0;
	float SightStrength = 0.f;
	
	bool bOverlapped = GetWorld()->OverlapMultiByObjectType(Overlaps, Location, Rotation, CollisionObjectQueryParams, OverlapShape, CollisionQueryParams);
	if (!bOverlapped)
		return nullptr;

	float BestDP = -FLT_MAX;
	float BestTargetDistanceSq = FLT_MAX;
	AActor* BestTarget = nullptr;

	FCanBeSeenFromContext CanBeSeenFromContext;
	CanBeSeenFromContext.IgnoreActor = GetOwner();
	CanBeSeenFromContext.ObserverLocation = OwnerEyesLocation;
	
	for (const auto& Overlap : Overlaps)
	{
		auto Actor = Overlap.GetActor();
		auto TargetableActor = Cast<ITargetable>(Actor);
		if (!TargetableActor || !TargetableActor->CanTarget(OwnerLocal))
			continue;
		
		if (auto ObservableActor = Cast<IAISightTargetInterface>(Actor))
		{
			FVector SeenAtLocation;
			auto CanBeSeenFromResult = ObservableActor->CanBeSeenFrom(CanBeSeenFromContext, SeenAtLocation, TracesThisTick, TracesThisTick, SightStrength);
			if (CanBeSeenFromResult == UAISense_Sight::EVisibilityResult::Visible)
				CheckTargetPriority(OwnerEyesLocation, ViewDirection, BestDP, BestTargetDistanceSq, BestTarget, Actor, SeenAtLocation);
		}
		else
		{
			FHitResult HitResult;
			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, OwnerEyesLocation, Actor->GetActorLocation(), ECC_Visibility, CollisionQueryParams);
			if (bHit && HitResult.GetActor() == Actor)
				CheckTargetPriority(OwnerEyesLocation, ViewDirection, BestDP, BestTargetDistanceSq, BestTarget, Actor, HitResult.Location);
		}
	}

	if (BestTarget == nullptr)
		return nullptr;
	
	if (Target.IsValid())
		ClearTarget();
	
	Target = BestTarget;
	auto CombatAliveCreature = Cast<ICombatAliveCreature>(Target);
	if (CombatAliveCreature)
		CombatCreatureDeadEventDelegateHandle = CombatAliveCreature->OnCombatCreatureDeadEvent.AddUObject(this, &UTargetLockComponent::OnTrackedCreatureDead);

	auto TargetableActor = Cast<ITargetable>(BestTarget);
	TargetableActor->SetTargetLockedOn(true);
	SetComponentTickEnabled(true);
	
	return BestTarget;
}

void UTargetLockComponent::ClearTarget()
{
	if (!Target.IsValid())
		return;

	if (auto CombatAliveCreature = Cast<ICombatAliveCreature>(Target))
	{
		CombatAliveCreature->OnCombatCreatureDeadEvent.Remove(CombatCreatureDeadEventDelegateHandle);
		CombatCreatureDeadEventDelegateHandle.Reset();
	}
	
	auto TargetableActor = Cast<ITargetable>(Target);
	TargetableActor->SetTargetLockedOn(false);
	
	Target.Reset();
	SetComponentTickEnabled(false);

	OnTargetLost.ExecuteIfBound();
}

void UTargetLockComponent::OnTrackedCreatureDead(AActor* Actor)
{
	ClearTarget();
}

void UTargetLockComponent::CheckTargetPriority(const FVector& OwnerEyesLocation, const FVector& ViewDirection, float& BestDP, float& BestTargetDistanceSq, AActor*& BestTarget, AActor* TestTarget, const
                                               FVector& SeenAtLocation)
{
	float DotProduct = ViewDirection | (SeenAtLocation - OwnerEyesLocation).GetSafeNormal();
	if (!ensure(DotProduct > DotProductThreshold)) // shouldn't even happen since I'm doing overlap test in front of the owner
		return;

	const float DistanceSq = (SeenAtLocation - OwnerEyesLocation).SizeSquared();
	if (DotProduct > BestDP)
	{
		constexpr float DotProductDiffThreshold = 0.025f; // approximately 10 degrees
		// if angle diff between best and this is not so much and the best is closer
		if (DotProduct - BestDP < DotProductDiffThreshold && BestTargetDistanceSq <= DistanceSq)
			return;

		BestTargetDistanceSq = DistanceSq;
		BestTarget = TestTarget;
		BestDP = DotProduct;
	}
}
