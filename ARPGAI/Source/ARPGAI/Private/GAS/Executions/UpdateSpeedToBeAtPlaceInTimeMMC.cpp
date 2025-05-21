// 


#include "GAS/Executions/UpdateSpeedToBeAtPlaceInTimeMMC.h"

#include "AIController.h"
#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Navigation/PathFollowingComponent.h"

float UUpdateSpeedToBeAtPlaceInTimeMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UUpdateSpeedToBeAtPlaceInTimeMMC::CalculateBaseMagnitude_Implementation)
	
	const float* BeAtLocationInTime = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_UpdateSpeedToBeAtLocationMMC_Time);
	if (!ensure(BeAtLocationInTime != nullptr))
		return 0.f;
	
	auto InstigatorActor = Spec.GetEffectContext().GetInstigator();
	auto NpcPawn = Cast<APawn>(InstigatorActor);
	if (!ensure(NpcPawn))
		return 0.f;

	auto AIController = Cast<AAIController>(NpcPawn->GetController());
	if (!ensure(AIController))
		return 0.f;

	FVector PathDestination = AIController->GetPathFollowingComponent()->GetPathDestination();
	if (PathDestination == FVector::ZeroVector || PathDestination == FAISystem::InvalidLocation)
	{
		UE_VLOG(NpcPawn, LogARPGAI, Warning, TEXT("UpdateSpeedToBeAtPlaceInTimeMMC: NPC's path destination is invalid"));
		return 0.f;
	}
	
	const float* MinSpeedPtr = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_UpdateSpeedToBeAtLocationMMC_MinSpeed);
	const float MinSpeed = MinSpeedPtr ? *MinSpeedPtr : 100.f;
	const float* MaxSpeedPtr = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_UpdateSpeedToBeAtLocationMMC_MaxSpeed);
	const float MaxSpeed = MaxSpeedPtr ? *MaxSpeedPtr : 1000.f;

	const float CurrentSpeed = NpcPawn->GetVelocity().Size();
	const float CatchUpTargetSpeed = (PathDestination - NpcPawn->GetActorLocation()).Size() / (*BeAtLocationInTime);
	const float ClampedCatchUpSpeed = FMath::Clamp(CatchUpTargetSpeed, MinSpeed, MaxSpeed);
	UE_VLOG(NpcPawn, LogARPGAI, VeryVerbose, TEXT("UpdateSpeedToBeAtPlaceInTimeMMC: Updated speed from %.2f to %.2f"), CurrentSpeed, ClampedCatchUpSpeed);
	return ClampedCatchUpSpeed;
}
