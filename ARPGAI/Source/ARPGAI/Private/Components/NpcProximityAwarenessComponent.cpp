#include "Components/NpcProximityAwarenessComponent.h"

#include "GameplayTagAssetInterface.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcAttitudesComponent.h"
#include "Interfaces/NpcAliveCreature.h"

UNpcProximityAwarenessComponent::UNpcProximityAwarenessComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SphereRadius = 300.f;
}

void UNpcProximityAwarenessComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
}

void UNpcProximityAwarenessComponent::Deactivate()
{
	Super::Deactivate();
	SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void UNpcProximityAwarenessComponent::BeginPlay()
{
	Super::BeginPlay();
	SetCollisionProfileName(AwarenessCollisionProfileName);
	OnComponentBeginOverlap.AddDynamic(this, &UNpcProximityAwarenessComponent::OnActorEnterProximity);
	OnComponentEndOverlap.AddDynamic(this, &UNpcProximityAwarenessComponent::OnActorExitProximity);
	Deactivate();
}

bool UNpcProximityAwarenessComponent::CanDetect(AActor* Actor) const
{
	auto PawnOwner = Cast<APawn>(GetOwner());
	if (Actor == PawnOwner)
		return false; // wtf?
	
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

	if (auto AttitudesComponent = GetNpcAttitudesComponent(PawnOwner))
		if (AttitudesComponent->IsFriendly(Actor))
			return false;
	
	auto AliveCreature = Cast<INpcAliveCreature>(Actor);
	if (AliveCreature && !AliveCreature->IsNpcActorAlive())
		return false;
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	TArray<AActor*> IgnoredActors = { PawnOwner, Actor };
	QueryParams.AddIgnoredActors(IgnoredActors);
	bool bCanTrace = !GetWorld()->LineTraceSingleByChannel(HitResult, GetOwner()->GetActorLocation(), Actor->GetActorLocation(),
		ECC_Visibility, QueryParams);

	return bCanTrace;
}

void UNpcProximityAwarenessComponent::OnActorEnterProximity(UPrimitiveComponent* OverlappedComponent,
                                                            AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                            const FHitResult& SweepResult)
{
	if (CanDetect(OtherActor))
		ActorsInProximity.Add(OtherActor);
}

void UNpcProximityAwarenessComponent::OnActorExitProximity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ActorsInProximity.Remove(OtherActor);
}
