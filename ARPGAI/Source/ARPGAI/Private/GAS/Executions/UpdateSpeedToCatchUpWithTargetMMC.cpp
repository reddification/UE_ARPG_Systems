#include "GAS/Executions/UpdateSpeedToCatchUpWithTargetMMC.h"
#include "Components/NpcComponent.h"
#include "Components/NpcInterfaceComponent.h"
#include "Data/AIGameplayTags.h"

#include "Interfaces/Npc.h"

UUpdateSpeedToCatchUpWithTargetMMC::UUpdateSpeedToCatchUpWithTargetMMC()
{
	DistanceToTargetDef.AttributeToCapture = UNpcCombatAttributeSet::GetDistanceToTargetAttribute();
	DistanceToTargetDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	DistanceToTargetDef.bSnapshot = false;
	
	RelevantAttributesToCapture.Add(DistanceToTargetDef);
}

float UUpdateSpeedToCatchUpWithTargetMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	auto InstigatorActor = Spec.GetEffectContext().GetInstigator();
	if (!InstigatorActor) return 0.f;
	
	auto Npc = Cast<INpc>(InstigatorActor);
	if (!ensure(Npc))
		return 0.f;

	auto NpcInterfaceComponent = InstigatorActor->FindComponentByClass<UNpcInterfaceComponent>();
	const AActor* CatchUpTarget = Npc->GetCatchUpTarget();
	if (ensure(!CatchUpTarget))
		return NpcInterfaceComponent->GetBaseMoveSpeed();
	
	const float* MinSpeedPtr = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_Speed_Min);
	const float MinSpeed = MinSpeedPtr ? *MinSpeedPtr : MinSpeedDefault;
	const float* MaxSpeedPtr = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_Speed_Max);
	const float MaxSpeed = MaxSpeedPtr ? *MaxSpeedPtr : MaxSpeedDefault;
	const float* SpeedScalePtr = Spec.SetByCallerTagMagnitudes.Find(AIGameplayTags::AI_SetByCaller_Speed_Scale);
	const float SpeedScale = SpeedScalePtr ? *SpeedScalePtr : SpeedScaleDefault;
	
	auto CatchUpTargetSpeed = CatchUpTarget->GetVelocity().Size();
	float CatchUpSpeed = FMath::Clamp(CatchUpTargetSpeed * SpeedScale, MinSpeed, MaxSpeed);
	return CatchUpSpeed;
}
