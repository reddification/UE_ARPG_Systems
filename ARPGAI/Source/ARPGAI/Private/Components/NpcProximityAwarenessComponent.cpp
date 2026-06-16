#include "Components/NpcProximityAwarenessComponent.h"

#include "GameplayTagAssetInterface.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcAttitudesComponent.h"
#include "Engine/OverlapResult.h"
#include "Interfaces/NpcAliveActor.h"

UNpcProximityAwarenessComponent::UNpcProximityAwarenessComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	OverlapDelegate = FOverlapDelegate::CreateUObject(this, &UNpcProximityAwarenessComponent::OnAsyncOverlapCompleted);
}

void UNpcProximityAwarenessComponent::BeginPlay()
{
	Super::BeginPlay();
	CollisionQueryParams.AddIgnoredActor(GetOwner());
}

void UNpcProximityAwarenessComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	GetWorld()->AsyncOverlapByObjectType(GetOwner()->GetActorLocation(),  FQuat::Identity, 
		CollisionObjectQueryParams, FCollisionShape::MakeSphere(OverlapRadius), CollisionQueryParams, &OverlapDelegate);
}

void UNpcProximityAwarenessComponent::ActivateProximityAwareness(float Radius, float UpdateInterval, const TArray<TEnumAsByte<ECollisionChannel>>& ObjectTypes,
                                                                 bool bInIgnoreAllies, const FGameplayTagQuery* OptionalDetectionBlockedFIlter)
{
	CollisionObjectQueryParams = FCollisionObjectQueryParams();
	for (const auto& ObjectType : ObjectTypes)
		CollisionObjectQueryParams.AddObjectTypesToQuery(ObjectType.GetValue());
	
	OverlapRadius = Radius;
	ActorsInProximity.Reserve(12);
	if (OptionalDetectionBlockedFIlter)
		DetectionBlockedTagQuery = *OptionalDetectionBlockedFIlter;
	
	bIgnoreAllies = bInIgnoreAllies;
	
	SetComponentTickEnabled(true);
	SetComponentTickInterval(UpdateInterval);
}

void UNpcProximityAwarenessComponent::DisableProximityAwareness()
{
	SetComponentTickEnabled(false);
	ActorsInProximity.Empty();
}

bool UNpcProximityAwarenessComponent::CanDetect(AActor* Actor) const
{
	auto PawnOwner = Cast<APawn>(GetOwner());
	if (Actor == PawnOwner)
		return ensure(false); // wtf?
	
	if (!DetectionBlockedTagQuery.IsEmpty())
	{
		if (auto GameplayTagActor = Cast<IGameplayTagAssetInterface>(Actor))
		{
			FGameplayTagContainer ActorTags;
			GameplayTagActor->GetOwnedGameplayTags(ActorTags);
			if (DetectionBlockedTagQuery.Matches(ActorTags))
				return false;
		}
	}

	if (bIgnoreAllies)
		if (auto AttitudesComponent = GetNpcAttitudesComponent(PawnOwner))
			if (AttitudesComponent->IsFriendly(Actor))
				return false;
	
	auto AliveCreature = Cast<INpcAliveActor>(Actor);
	if (AliveCreature && !AliveCreature->IsAlive_NPC())
		return false;
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	TArray<AActor*> IgnoredActors = { PawnOwner, Actor };
	QueryParams.AddIgnoredActors(IgnoredActors);
	bool bCanTrace = !GetWorld()->LineTraceSingleByChannel(HitResult, GetOwner()->GetActorLocation(), Actor->GetActorLocation(),
		ECC_Visibility, QueryParams);

	return bCanTrace;
}

void UNpcProximityAwarenessComponent::OnActorEnterProximity(AActor* OtherActor)
{
	if (CanDetect(OtherActor))
		ActorsInProximity.Add(OtherActor);
}

void UNpcProximityAwarenessComponent::OnActorExitProximity(AActor* OtherActor)
{
	ActorsInProximity.Remove(OtherActor);
}

void UNpcProximityAwarenessComponent::OnAsyncOverlapCompleted(const FTraceHandle& TraceHandle, FOverlapDatum& OverlapDatum)
{
	if (!TraceHandle.IsValid())
		return;
	
	TArray<AActor*, TInlineAllocator<8>> PerceivedActors;
	for (const auto& OverlapResult : OverlapDatum.OutOverlaps)
	{
		auto OverlappedActor =OverlapResult.GetActor();
		if (CanDetect(OverlappedActor))
			PerceivedActors.Add(OverlappedActor);
	}
	
	TArray<TWeakObjectPtr<AActor>, TInlineAllocator<12>> OldActorsInProximity;
	ActorsInProximity.GetKeys(OldActorsInProximity);
	
	for (const auto& IsInProximityTest : OldActorsInProximity)
		if (!IsInProximityTest.IsValid() || !PerceivedActors.Contains(IsInProximityTest))
			ActorsInProximity.Remove(IsInProximityTest);
	
	for (const auto& PerceivedActor : PerceivedActors)
	{
		auto& ProximityData = ActorsInProximity.FindOrAdd(PerceivedActor);
		ProximityData.Duration += GetComponentTickInterval();
	}
}
