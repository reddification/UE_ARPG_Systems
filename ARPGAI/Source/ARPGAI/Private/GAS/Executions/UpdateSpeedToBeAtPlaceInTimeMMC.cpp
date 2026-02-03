// 


#include "GAS/Executions/UpdateSpeedToBeAtPlaceInTimeMMC.h"

#include "AIController.h"
#include "Components/NpcComponent.h"
#include "Components/NpcInterfaceComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

UUpdateSpeedToBeAtPlaceInTimeMMC::UUpdateSpeedToBeAtPlaceInTimeMMC()
{
	DistanceToTargetDef.AttributeToCapture = UNpcCombatAttributeSet::GetDistanceToTargetAttribute();
	DistanceToTargetDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	DistanceToTargetDef.bSnapshot = false;
	
	RelevantAttributesToCapture.Add(DistanceToTargetDef);
}

float UUpdateSpeedToBeAtPlaceInTimeMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UUpdateSpeedToBeAtPlaceInTimeMMC::CalculateBaseMagnitude_Implementation)
	
	const float* BeAtLocationInTime = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_Time);
	if (!ensure(BeAtLocationInTime != nullptr))
		return 0.f;
	
	auto InstigatorActor = Spec.GetEffectContext().GetInstigator();
	auto NpcPawn = Cast<APawn>(InstigatorActor);
	if (!ensure(NpcPawn))
		return 0.f;

	auto BaseSpeed = NpcPawn->GetMovementComponent()->GetMaxSpeed();
	if (auto NpcInterfaceComponent = NpcPawn->FindComponentByClass<UNpcInterfaceComponent>())
		BaseSpeed = NpcInterfaceComponent->GetBaseMoveSpeed();
	
	auto AIController = Cast<AAIController>(NpcPawn->GetController());
	if (!ensure(AIController))
		return BaseSpeed;

	FVector PathDestination = AIController->GetPathFollowingComponent()->GetPathDestination();
	if (PathDestination == FVector::ZeroVector || PathDestination == FAISystem::InvalidLocation)
	{
		UE_VLOG(NpcPawn, LogARPGAI, Warning, TEXT("UpdateSpeedToBeAtPlaceInTimeMMC: NPC's path destination is invalid"));
		return BaseSpeed;
	}
	
	const float* MinSpeedPtr = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_Speed_Min);
	const float MinSpeed = MinSpeedPtr ? *MinSpeedPtr : MinSpeedDefault;
	const float* MaxSpeedPtr = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_Speed_Max);
	const float MaxSpeed = MaxSpeedPtr ? *MaxSpeedPtr : MaxSpeedDefault;

	const float CurrentSpeed = NpcPawn->GetVelocity().Size();
	const float CatchUpTargetSpeed = (PathDestination - NpcPawn->GetActorLocation()).Size() / (*BeAtLocationInTime);
	const float ClampedCatchUpSpeed = FMath::Clamp(CatchUpTargetSpeed, MinSpeed, MaxSpeed);
	UE_VLOG(NpcPawn, LogARPGAI, VeryVerbose, TEXT("UpdateSpeedToBeAtPlaceInTimeMMC: Updated speed from %.2f to %.2f"), CurrentSpeed, ClampedCatchUpSpeed);
	
	return ClampedCatchUpSpeed;
}
