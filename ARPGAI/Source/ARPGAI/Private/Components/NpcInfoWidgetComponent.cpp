// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/NpcInfoWidgetComponent.h"

#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "GameFramework/Character.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Npc.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/NpcCombatSettings.h"
#include "Widgets/NpcStateWidget.h"

// Sets default values for this component's properties
UNpcInfoWidgetComponent::UNpcInfoWidgetComponent()
{
	Space = EWidgetSpace::Screen;	
}

// Called when the game starts
void UNpcInfoWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNpcInfoWidgetComponent::InitializeNpc);
}

void UNpcInfoWidgetComponent::InitializeNpc()
{
	auto NpcCombatSettings = GetDefault<UNpcCombatSettings>();
	if (NpcCombatSettings->NpcInfoWidgetClass)
	{
		SetWidgetClass(NpcCombatSettings->NpcInfoWidgetClass);
		NpcStateWidget = Cast<UNpcStateWidget>(GetWidget());
	}
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, NpcCombatSettings->NpcInfoWidgetSocketName);

	if (auto AliveCreature = Cast<INpcAliveCreature>(GetOwner()))
	{
		AliveCreature->OnDeathStarted.AddUObject(this, &UNpcInfoWidgetComponent::OnDeathStarted);
	}

	NpcStateWidget = Cast<UNpcStateWidget>(GetWidget());
	if (!NpcStateWidget.IsValid())
		return;

	NpcStateWidget->SetNPC(OwnerCharacter);
	NpcStateWidget->SetDetailedNpcView(bDisplayDetailedNpcView);
	
	PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	OwnerNpcComponent = OwnerCharacter->FindComponentByClass<UNpcComponent>();
	MinDotProductToShowWidget = NpcCombatSettings->MinPlayerToNpcDotProductToShowWidget;
	ConsiderableDistanceToPlayerForHostile = NpcCombatSettings->NpcInfoWidgetConsiderableDistanceToPlayer;
	
	GetWorld()->GetTimerManager().SetTimer(UpdateVisibilityTimer, this, &UNpcInfoWidgetComponent::UpdateVisibility,
		NpcCombatSettings->NpcInfoWidgetVisibilityUpdateInterval, true);

	SetVisibility(false);
}

// void UNpcInfoWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
//                                             FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
// 	if (bScaleWidget)
// 	{
// 		float DistanceToPlayerSq = (GetOwner()->GetActorLocation() - PlayerPawn->GetActorLocation()).SizeSquared();
// 		SetScale
// 	}
// }

void UNpcInfoWidgetComponent::UpdateVisibility()
{
	if (!OwnerNpcComponent.IsValid())
		return;

	if (!PlayerPawn.IsValid())
		return;
	
	FGameplayTag AttitudeToPlayer = OwnerNpcComponent->GetAttitude(PlayerPawn.Get());

	bool bHostile = AttitudeToPlayer.MatchesTag(AIGameplayTags::AI_Attitude_Hostile);
	float ConsiderableDistance = bHostile ? ConsiderableDistanceToPlayerForHostile : ConsiderableDistanceToPlayerForNonHostile;

	NpcStateWidget->SetHostile(bHostile);
	
	if (PlayerPawn.IsValid())
	{
		float DistanceToPlayerSq = (GetOwner()->GetActorLocation() - PlayerPawn->GetActorLocation()).SizeSquared();
		if (DistanceToPlayerSq < ConsiderableDistance * ConsiderableDistance)
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			PlayerPawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
			if ((ViewRotation.Vector() | (GetOwner()->GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal()) > MinDotProductToShowWidget)
			{
				FHitResult HitResult;
				FCollisionQueryParams CollisionQueryParams;
				CollisionQueryParams.AddIgnoredActor(GetOwner());
				const FVector StartLocation = GetOwner()->GetActorLocation();
				const FVector EndLocation = PlayerPawn->GetActorLocation();
				bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation + (EndLocation - StartLocation).GetSafeNormal() * 50.f, ECC_Visibility,
					CollisionQueryParams);

				if (bHit && HitResult.GetActor() == PlayerPawn)
				{
					SetVisibility(true);
					return;
				}
			}
		}
	}

	SetVisibility(false);
}

void UNpcInfoWidgetComponent::OnDeathStarted(AActor* OwningActor)
{
	SetVisibility(false);
}
