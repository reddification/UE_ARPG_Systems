#include "Components/NpcProximityAwarenessComponent.h"

#include "GameplayTagAssetInterface.h"

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
	SetCollisionProfileName("NpcAwareness");
	OnComponentBeginOverlap.AddDynamic(this, &UNpcProximityAwarenessComponent::OnActorEnterProximity);
	OnComponentEndOverlap.AddDynamic(this, &UNpcProximityAwarenessComponent::OnActorExitProximity);
	Deactivate();
}

bool UNpcProximityAwarenessComponent::CanDetect(AActor* Actor) const
{
	if (Actor == GetOwner())
		return false;
	
	if (DetectionBlockedTagQuery.IsEmpty())
		return true;
	
	if (auto GameplayTagActor = Cast<IGameplayTagAssetInterface>(Actor))
	{
		FGameplayTagContainer ActorTags;
		GameplayTagActor->GetOwnedGameplayTags(ActorTags);
		return !DetectionBlockedTagQuery.Matches(ActorTags);
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	TArray<AActor*> IgnoredActors = { GetOwner(), Actor };
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
